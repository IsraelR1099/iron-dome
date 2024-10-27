#include "notify.h"

void	create_dir(char *dir)
{
	char	error_msg[2048] = {0};
	char	temp[1024] = {0};
	char	*pos;
	size_t	len;

	snprintf(temp, sizeof(temp), "%s", dir);
	len = strlen(temp);
	if (temp[len - 1] == '/')
		temp[len - 1] = '\0';
	for (pos = temp + 1; *pos; pos++)
	{
		if (*pos == '/')
		{
			*pos = '\0';
			if (mkdir(temp, 0755) < 0)
			{
				if (errno != EEXIST)
				{
					snprintf(error_msg, sizeof(error_msg), "mkdir error: %s", temp);
					perror(error_msg);
					exit (1);
				}
			}
			*pos = '/';
		}
	}
	if (mkdir(temp, 0755) < 0)
	{
		if (errno != EEXIST)
		{
			snprintf(error_msg, sizeof(error_msg), "mkdir error: %s", temp);
			perror(error_msg);
			exit (1);
		}
	}
}

bool	endswith(char *s, char *suffix)
{
	int	s_len;
	int	suffix_len;

	s_len = strlen(s);
	suffix_len = strlen(suffix);
	if (s_len < suffix_len)
		return (false);
	return (strcmp(s + s_len - suffix_len, suffix) == 0);
}
