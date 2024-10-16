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
		perror("open error get_uptime");
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
	close(fd);
	return (atof(buf));
}

char	*get_proc_stat(int pid)
{
	int		fd;
	char	path[256];
	char	buf[256];
	char	*ret;
	int		read_bytes;

	snprintf(path, sizeof(path), "/proc/%d/stat", pid);
	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		perror("open error get_proc");
		exit (1);
	}
	read_bytes = read(fd, buf, 256);
	if (read_bytes < 0)
	{
		perror("read error");
		exit (1);
	}
	buf[read_bytes] = '\0';
	ret = strdup(buf);
	close(fd);
	return (ret);
}

char	*get_word(char *str)
{
	int		i;
	int		j;
	char	*word;

	i = 0;
	while (str[i] == ' ')
		i++;
	j = i;
	while (str[j] != ' ')
		j++;
	word = (char *)calloc(j - i + 1, sizeof(char));
	if (!word)
	{
		perror("calloc error");
		exit (1);
	}
	strncpy(word, &str[i], j - i);
	return (word);
}

int	get_time(char *buf, int index)
{
	int		i;
	int		count;
	int		ret;
	char	*tmp;

	count = 0;
	for (i = 0; buf[i]; i++)
	{
		if (buf[i] == ' ')
			count++;
		if (count == index)
		{
			tmp = get_word(&buf[i]);
			break ;
		}
	}
	ret = atoi(tmp);
	free(tmp);
	tmp = NULL;
	return (ret);
}


float	calculate_cpu_usage(int pid)
{
	float	uptime;
	int	utime;
	int	stime;
	int	cutime;
	int	cstime;
	int	starttime;
	int	hertz;
	int	total_time;
	int	seconds;
	int	percent;
	char	*buf;

	uptime = get_uptime();
//	printf("uptime es %f\n", uptime);
	buf = get_proc_stat(pid);
//	printf("buf en calculate %s\n", buf);
	utime = get_time(buf, 13);
//	printf("utime es %d\n", utime);
	stime = get_time(buf, 14);
//	printf("stime es %d\n", stime);
	cutime = get_time(buf, 15);
//	printf("cutime es %d\n", cutime);
	cstime = get_time(buf, 16);
//	printf("cstime es %d\n", cstime);
	starttime = get_time(buf, 21);
//	printf("starttime es %d\n", starttime);
	hertz = sysconf(_SC_CLK_TCK);
//	printf("hertz es %d\n", hertz);
	total_time = utime + stime;
	total_time = total_time + cutime + cstime;
//	printf("total_time es %d\n", total_time);
	seconds = uptime - (starttime / hertz);
//	printf("seconds es %d\n", seconds);
	if (seconds == 0)
		return (0);
	percent = 100 * ((total_time / hertz) / seconds);
//	printf("percent es %d\n", percent);
	free(buf);
	return(percent);

}
/*
int	main(int argc, char **argv)
{
	float	cpu_usage;
	int		pid;

	pid = atoi(argv[1]);
	cpu_usage = calculate_cpu_usage(pid);
	printf("cpu_usage es %f\n", cpu_usage);
	(void)argc;
	return (0);
}*/
