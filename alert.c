#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

extern pthread_mutex_t	mutex;

void	alert(const char *msg)
{
	FILE	*log_file;

	pthread_mutex_lock(&mutex);
	log_file = fopen("/var/log/irondome.log", "a");
	if (log_file)
	{
		fprintf(log_file, "%s", msg);
		fclose(log_file);
	}
	else
	{
		fprintf(stderr, "Failed to open irondome.log\n");
		printf("Failure %s\n", strerror(errno));
		exit(1);
	}
	pthread_mutex_unlock(&mutex);
}
