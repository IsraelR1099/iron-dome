#include <errno.h>
#include <signal.h>
#include "notify.h"

#define BUF_SIZE 512

extern float                    calculate_cpu_usage(int pid);
extern void                     mask_signals(void);
extern volatile sig_atomic_t    stop;
extern void                     alert(const char *message);

static void check_crypto_libs(int pid)
{
	char	path[BUF_SIZE] = {0};
	char	line[BUF_SIZE] = {0};
	FILE	*file;
	float	cpu_usage;
	char	buffer[1024];

	snprintf(path, sizeof(path), "/proc/%d/maps", pid);
	memset(buffer, 0, sizeof(buffer));
	file = fopen(path, "r");
	if (!file)
	{
		perror("fopen error");
		printf("%s\n", strerror(errno));
		exit(1);
	}
	while (fgets(line, sizeof(line), file))
	{
		if (strstr(line, "libcrypto")
				|| strstr(line, "libssl")
				|| strstr(line, "libcrypt")
				|| strstr(line, "openssl")
				|| strstr(line, "gpg")
				|| strstr(line, "python"))
		{
			//printf("INFO: Process %d is using crypto libraries: %s", pid, line);
			cpu_usage = calculate_cpu_usage(pid);
			//printf("cpu_usage es %f\n", cpu_usage);
			if (cpu_usage > 5.0)
            		{
				printf("\033[1;31mWARNING: Process %d is using a lot of CPU and crypto libraries: %f\033[0m\n", pid, cpu_usage);
				snprintf(buffer, sizeof(buffer), "WARNING: Process %d is using a lot of CPU and crypto libraries: %f\n", pid, cpu_usage);
				snprintf(buffer, sizeof(buffer), "WARNING: The followed process is using a lot of CPU and crypto libraries: %d, %f\n", pid, cpu_usage);
				alert(buffer);
			}
		}
	}
	fclose(file);
}

void	*scan_processes(void *arg)
{
	DIR             *dir;
	struct dirent   *entry;
	int             pid;

	mask_signals();
	while (!stop)
	{
		dir = opendir("/proc");
		if (!dir)
		{
			perror("opendir error");
			exit(1);
		}
		while ((entry = readdir(dir)) != NULL)
		{
			if (entry->d_type == DT_DIR)
			{
				pid = atoi(entry->d_name);
				if (pid > 0)
					check_crypto_libs(pid);
			}
		}
		closedir(dir);
		sleep(30);
	}
	(void)arg;
	printf("scan_processes thread is exiting\n");
	return (NULL);
}
