#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <poll.h>
#include <string.h>
#include <signal.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))
#define READ_THRESHOLD 10
volatile sig_atomic_t stop = 0;

static void	terminate_handler(int signum)
{
	printf("Terminating...\n");
	(void)signum;
	stop = 1;
}

static void	handle_events(int fd, int *wd, int *read_count, int argc, char *argv[])
{
	char	buf[BUF_LEN]
		__attribute__ ((aligned(__alignof__(struct inotify_event))));;
	const struct inotify_event	*event;
	ssize_t				len;

	//Loop while events can be read from inotify file descriptor
	for (;;)
	{
		len = read(fd, buf, sizeof(buf));
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
		//Print the name of the watched directory
		for (size_t i = 1; i < (size_t)argc; i++)
		{
			if (wd[i] == event->wd)
			{
				printf("Watching %s and read %d\n", argv[i], read_count[i]);
				read_count[i]++;
				if (read_count[i] > READ_THRESHOLD)
					printf("WARNING: Possible disk read abuse detected.\n");
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

int	notify(int argc, char **argv)
{
	char			buf;
	int			fd;
	int			poll_num;
	int			i;
	int			*wd;
	int			*read_count;
	nfds_t			nfds;
	struct	pollfd		fds[2];
	struct sigaction	sa;
	struct sigaction	so;

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
	sa.sa_handler = terminate_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, NULL, &so);
	if (so.sa_handler != SIG_IGN)
		sigaction(SIGINT, &sa, NULL);
	//Wait for events and/or terminal input
	printf("Listening for events.\n");
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
	printf("Listening for events stopped.\n");
	//Clean up
	for (i = 1; i < argc; i++)
		inotify_rm_watch(fd, wd[i]);
	close(fd);
	free(wd);
	free(read_count);
	printf("Resources cleaned up. Exiting.\n");
	return (0);
}
