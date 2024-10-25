#include "notify.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

pthread_mutex_t			mutex = PTHREAD_MUTEX_INITIALIZER;
volatile sig_atomic_t	stop = 0;
extern void				*backup(void *argv);
extern void				mask_signals(void);

int	main(int argc, char **argv)
{
	pthread_t			thread1;
	struct inotify_args	args;
	int					ret;


	args.argc = argc;
	args.argv = argv;
	args.bk_interval = 15;
	ret = pthread_create(&thread1, NULL, backup, &args);
	if (ret)
	{
		fprintf(stderr, "Error pthread_create code %d\n", ret);
		exit (EXIT_FAILURE);
	}
	pthread_join(thread1, NULL);
	return (0);
}
