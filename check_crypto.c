#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 512

extern float	calculate_cpu_usage(int pid);

static void	check_crypto_libs(int pid)
{
	char	path[BUF_SIZE];
	char	line[BUF_SIZE];
	FILE	*file;
	float	cpu_usage;

	snprintf(path, sizeof(path), "/proc/%d/maps", pid);
	printf("Checking process %d\n", pid);
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
				|| strstr(line, "gpg"))
		{
			printf("Process %d is using crypto libraries: %s", pid, line);
			cpu_usage = calculate_cpu_usage(pid);
			if (cpu_usage > 0.5)
				printf("Process %d is using a lot of CPU: %f\n", pid, cpu_usage);
		}
	}
	fclose(file);
}

void	scan_processes(void)
{
	DIR		*dir;
	struct dirent	*entry;
	int		pid;

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
}

int	main(void)
{
	scan_processes();
	return (0);
}
