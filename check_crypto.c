#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define BUF_SIZE 512

extern float                    calculate_cpu_usage(int pid);
extern void                     mask_signals(void);
extern volatile sig_atomic_t    stop;
extern void                     alert(const char *message);

static void check_crypto_libs(int pid)
{
	char	path[BUF_SIZE];
	char	line[BUF_SIZE];
	FILE	*file;
	float	cpu_usage;
	char	buffer[100];
	char	info[1024];

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
        memset(info, 0, sizeof(info));
		if (strstr(line, "libcrypto")
				|| strstr(line, "libssl")
				|| strstr(line, "libcrypt")
				|| strstr(line, "openssl")
				|| strstr(line, "gpg"))
		{
			printf("INFO: Process %d is using crypto libraries: %s", pid, line);
            snprintf(info, sizeof(info), "INFO: Process %d is using crypto libraries: %s", pid, line);
            alert(info);
			cpu_usage = calculate_cpu_usage(pid);
			if (cpu_usage > 0.5)
            {
                snprintf(buffer, sizeof(buffer), "WARNING: Process %d is using a lot of CPU: %f\n", pid, cpu_usage);
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
