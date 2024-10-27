#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>
#include "notify.h"

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))
#define READ_THRESHOLD 10
volatile sig_atomic_t	stop = 0;
extern void		*scan_processes(void *arg);
extern void		mask_signals(void);
extern void		*entropy(void *argv);
extern void		*backup(void *argv);
extern void		alert(const char *message);
pthread_mutex_t		mutex = PTHREAD_MUTEX_INITIALIZER;


static void	terminate_handler(int signum)
{
	printf("Pressed Ctrl+C\n");
	(void)signum;
	stop = 1;
}

static void	handle_events(int fd, int *wd, int *read_count, int argc, char *argv[])
{
	char	buf[BUF_LEN]
		__attribute__ ((aligned(__alignof__(struct inotify_event))));;
	const struct inotify_event	*event;
	ssize_t				len;
	time_t				start_time;
	int				time_window;
	char				buffer[100];

	start_time = time(NULL);
	time_window = 30;
	//Loop while events can be read from inotify file descriptor
	for (;;)
	{
		len = read(fd, buf, sizeof(buf));
		memset(buffer, 0, 100);
		if (len < 0 && errno != EAGAIN)
		{
			if (errno == EINTR)
			{
				if (stop)
					break ;
				continue ;
			}
			perror("read error");
			exit(EXIT_FAILURE);
		}
		if (len <= 0)
			break ;
		for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len)
		{
			event = (const struct inotify_event *)ptr;
			if (event->mask & IN_OPEN)
				printf("File %s was opened\n", event->name);
			if (event->mask & IN_CLOSE_NOWRITE)
				printf("File %s was closed\n", event->name);
			if (event->mask & IN_CLOSE_WRITE)
				printf("File %s was written\n", event->name);
			if (event->mask & IN_ACCESS)
				printf("File %s was accessed\n", event->name);
		}
		for (size_t i = 1; i < (size_t)argc; i++)
		{
			if (wd[i] == event->wd)
			{
				read_count[i]++;
				if (time(NULL) - start_time >= time_window)
				{
					if (read_count[i] > READ_THRESHOLD)
					{
						printf("\033[1;31mWARNING: Possible disk read abuse detected on file %s\033[0m\n", argv[i]);
						snprintf(buffer, sizeof(buffer), "WARNING: Possible disk read abuse detected on file %s\n", argv[i]);
						alert(buffer);
						start_time = time(NULL);
						read_count[i] = 0;
					}
				}
				break ;
			}
		}
		//Print the name of the file
		if (event->len)
			printf("File name: %s\n", event->name);
		//Print type of filesystem object
		if (event->mask & IN_ISDIR)
			printf("The object is a directory.\n");
		else
			printf("The object is a file.\n");
	}
}

static void	iterate_loop(struct pollfd *fds, nfds_t nfds, int fd, int *wd, int *read_count, int argc, char *argv[])
{
	int	poll_num;

	while (!stop)
	{
		poll_num = poll(fds, nfds, -1);
		if (poll_num < 0)
		{
			if (errno == EINTR)
				continue ;
			perror("poll");
			exit(EXIT_FAILURE);
		}
		if (poll_num > 0)
		{
			if (fds[0].revents & POLLIN)
			{
				//Console input is available. Empty stdin and quit
				char	buf;
				while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
					continue ;
				break ;
			}
			if (fds[1].revents & POLLIN)
			{
				//Inotify events are available
				handle_events(fd, wd, read_count, argc, argv);
			}
		}
	}
}

static void	set_signals(void)
{
	struct sigaction	sa;

	sa.sa_handler = terminate_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}

void	*notify(void *args)
{
	int			fd;
	int			i;
	int			*wd;
	int			*read_count;
	nfds_t			nfds;
	struct	pollfd		fds[2];
	struct inotify_args	*inotify_args = (struct inotify_args *)args;

	//Get arguments
	int argc = inotify_args->argc;
	char **argv = inotify_args->argv;

	if (argc < 2) {
		printf("Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	fd = inotify_init();
	if (fd < 0)
	{
		perror("inotify_init");
		exit(EXIT_FAILURE);
	}
	//Allocate memory for watch descriptors
	wd = (int *)calloc(argc, sizeof(int));
	read_count = (int *)calloc(argc, sizeof(int));
	if (wd == NULL || read_count == NULL)
	{
		perror("calloc");
		exit(EXIT_FAILURE);
	}
	for (i = 1; i < argc; i++)
	{
	wd[i] = inotify_add_watch(fd, argv[i], IN_MODIFY | IN_CLOSE | IN_OPEN | IN_ACCESS);
		if (wd[i] < 0)
		{
			perror("inotify_add_watch");
			exit(EXIT_FAILURE);
		}
	}
	//Prepare for polling
	nfds = 2;
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;
	fds[1].fd = fd;
	fds[1].events = POLLIN;

	//Set up signal handler
	//Wait for events and/or terminal input
	printf("Listening for events.\n");
	iterate_loop(fds, nfds, fd, wd, read_count, argc, argv);
	printf("Listening for events stopped.\n");
	//Clean up
	for (i = 1; i < argc; i++)
		inotify_rm_watch(fd, wd[i]);
	close(fd);
	free(wd);
	free(read_count);
	printf("\033[33mResources cleaned up. Exiting.\n\033[0m");
	return (NULL);
}

int	check_functions(int argc, char **argv, int backup_interval)
{
	pthread_t			thread1;
	pthread_t			thread2;
	pthread_t			thread3;
	pthread_t			thread4;
	pthread_attr_t		attr;
	int					ret;
	struct inotify_args	args;

	set_signals();
	printf("backup_interval: %d\n", backup_interval);
	args.argc = argc;
	args.argv = argv;
	args.bk_interval = backup_interval;
	pthread_attr_init(&attr);
	pthread_mutex_init(&mutex, NULL);
	ret = pthread_create(&thread1, &attr, notify, (void *)&args);
	if (ret)
	{
		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
		exit(EXIT_FAILURE);
	}
	ret = pthread_create(&thread2, NULL, scan_processes, NULL);
	if (ret)
	{
		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
		exit(EXIT_FAILURE);
	}
	ret = pthread_create(&thread3, NULL, entropy, (void *)&args);
	if (ret)
	{
		fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
		exit(EXIT_FAILURE);
	}
	if (backup_interval > 10)
	{
		ret = pthread_create(&thread4, NULL, backup, (void *)&args);
		if (ret)
		{
			fprintf(stderr, "Error - pthread_create() return code: %d\n", ret);
			exit(EXIT_FAILURE);
		}
	}
	mask_signals();
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	if (backup_interval > 10)
		pthread_join(thread4, NULL);
	pthread_mutex_destroy(&mutex);
	return (0);
}
