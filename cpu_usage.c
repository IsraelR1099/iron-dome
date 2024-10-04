#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

float	get_uptime(void)
{
	int				fd;
	int				i;
	char			buf[100];

	fd = open("/proc/uptime", O_RDONLY);
	if (fd < 0)
	{
		perror("open error");
		exit (1);
	}
	memset(buf, 0, sizeof(buf));
	read(fd, buf, sizeof(buf));
	for (i = 0; buf[i]; i++)
	{
		if (buf[i] == ' ')
			break ;
	}
	buf[i] = '\0';
	printf("buf es |%s|\n", buf);
	printf("i es %d\n", i);
	return (atof(buf));
}

char	*get_proc_stat(int pid)
{
	int		fd;
	char	path[256];
	char	*buf;
	int		read_bytes;

	snprintf(path, sizeof(path), "/proc/%d/stat", pid);
	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		perror("open error");
		exit (1);
	}
	buf = (char *)calloc(256, sizeof(char));
	if (!buf)
	{
		perror("calloc error");
		exit (1);
	}
	read_bytes = read(fd, buf, 256);
	if (read_bytes < 0)
	{
		perror("read error");
		exit (1);
	}
	buf[read_bytes] = '\0';
	return (buf);
}


float	calculate_cpu_usage(int pid)
{
	float			uptime;
	//unsigned int	utime;
	char			*buf;


	uptime = get_uptime();
	buf = get_proc_stat(pid);
	printf("buf en calculate %s\n", buf);
	free(buf);
	return(uptime);

}

int	main(int argc, char **argv)
{
	float	cpu_usage;
	int		pid;

	pid = atoi(argv[1]);
	cpu_usage = calculate_cpu_usage(pid);
	printf("cpu_usage es %f\n", cpu_usage);
	(void)argc;
	return (0);
}
