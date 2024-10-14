#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

extern pthread_mutex_t	mutex;

void	alert(const char *msg)
{
	FILE	*log_file;

	pthread_mutex_lock(&mutex);
	log_file = fopen("irondome.log", "a");
	if (log_file)
	{
		fprintf(log_file, "%s", msg);
		fclose(log_file);
	}
	else
	{
		fprintf(stderr, "Failed to open alert.log\n");
		exit(1);
	}
	pthread_mutex_unlock(&mutex);
}
