#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/stat.h>

extern void			mask_signals(void);
extern volatile sig_atomic_t	stop;
extern void			alert(const char *msg);

typedef struct s_file_entropy
{
	char	*file;
	float	baseline_entropy;
}	t_file_entropy;

static float	calc_entropy(char *str, unsigned char data[])
{
	unsigned char	count[256] = {0};
	int		len;
	float		entropy = 0;
	float		prob;

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; str[j]; j++)
		{
			if (data[i] == str[j])
				count[i]++;
		}
	}
	len = strlen(str);
	for (int i = 0; i < 256; i++)
	{
		if (count[i] != 0)
		{
			prob = (float)count[i] / len;
			entropy -= prob * log2(prob);
		}
	}
	return (entropy);
}

static void	iter_file(char *file, unsigned char data[], t_file_entropy *file_info)
{
	FILE	*fp;
	int	ret;
	char	buf[1024];
	float	current_entropy;
	float	entropy_change;
	float	percent_change;
	char	msg[1024];

	fp = fopen(file, "rb");
	entropy_change = 0.0;
	percent_change = 0.0;
	memset(msg, 0, sizeof(msg));
	printf("initial entropy of %s: %f\n", file, file_info->baseline_entropy);
	if (fp)
	{
		memset(buf, 0, sizeof(buf));
		ret = fread(buf, sizeof(char), sizeof(buf), fp);
		if (ret)
		{
			current_entropy = calc_entropy(buf, data);
			printf("Current entropy of %s: %f\n", file, current_entropy);
			if (file_info->baseline_entropy == -1)
				file_info->baseline_entropy = current_entropy;
			else
			{
				entropy_change = fabs(current_entropy - file_info->baseline_entropy);
				percent_change = (entropy_change / file_info->baseline_entropy) * 100;
				printf("Entropy change of %s: %f (%f%%)\n", file, entropy_change, percent_change);
				if (percent_change > 5.0)
				{
					printf("Warning: %s's entropy has changed by %f%%\n", file, percent_change);
					snprintf(msg, sizeof(msg), "Warning: %s's entropy has changed by %f%%\n", file, percent_change);
					alert(msg);
					file_info->baseline_entropy = current_entropy;
				}
			}
		}
		else
		{
			if (ferror(fp))
				perror("fread");
			else
				fprintf(stderr, "fread: unexpected EOF\n");
		}
	}
	else
	{
		perror("fopen");
	}
	fclose(fp);
}

static int	ft_len(void *argv)
{
	int	i;
	char	**files;

	files = (char **)argv;
	for (i = 0; files[i]; i++)
		;
	return (i);
}


void	*entropy(void *argv)
{
	unsigned char	data[] = {
		'\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
		'\x08', '\t', '\n', '\x0b', '\x0c', '\r', '\x0e', '\x0f',
		'\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17',
		'\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f',
		' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
		'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', '`',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|',
		'}', '~', '\x7f', '\x80', '\x81', '\x82', '\x83', '\x84', '\x85',
		'\x86', '\x87', '\x88', '\x89', '\x8a', '\x8b', '\x8c', '\x8d',
		'\x8e', '\x8f', '\x90', '\x91', '\x92', '\x93', '\x94', '\x95',
		'\x96', '\x97', '\x98', '\x99', '\x9a', '\x9b', '\x9c', '\x9d',
		'\x9e', '\x9f', '\xa0', '\xa1', '\xa2', '\xa3', '\xa4', '\xa5',
		'\xa6', '\xa7', '\xa8', '\xa9', '\xaa', '\xab', '\xac', '\xad',
		'\xae', '\xaf', '\xb0', '\xb1', '\xb2', '\xb3', '\xb4', '\xb5',
		'\xb6', '\xb7', '\xb8', '\xb9', '\xba', '\xbb', '\xbc', '\xbd',
		'\xbe', '\xbf', '\xc0', '\xc1', '\xc2', '\xc3', '\xc4', '\xc5',
		'\xc6', '\xc7', '\xc8', '\xc9', '\xca', '\xcb', '\xcc', '\xcd',
		'\xce', '\xcf', '\xd0', '\xd1', '\xd2', '\xd3', '\xd4', '\xd5',
		'\xd6', '\xd7', '\xd8', '\xd9', '\xda', '\xdb', '\xdc', '\xdd',
		'\xde', '\xdf', '\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5',
		'\xe6', '\xe7', '\xe8', '\xe9', '\xea', '\xeb', '\xec', '\xed',
		'\xee', '\xef', '\xf0', '\xf1', '\xf2', '\xf3', '\xf4', '\xf5',
		'\xf6', '\xf7', '\xf8', '\xf9', '\xfa', '\xfb', '\xfc', '\xfd',
		'\xfe', '\xff'
	};
	int		i;
	struct stat	st;
	char		**files;
	t_file_entropy	file_info[ft_len(argv)];

	mask_signals();
	files = (char **)argv;
	for (i = 1; files[i]; i++)
	{
		file_info[i - 1].file = files[i];
		file_info[i - 1].baseline_entropy = -1;
	}
	while (!stop)
	{
		for (i = 1; files[i]; i++)
		{
			if (stat(files[i], &st) == -1)
			{
				perror("stat error");
				exit (1);
			}
			if (S_ISDIR(st.st_mode))
			{
				fprintf(stderr, "%s: is a directory\n", files[i]);
				continue ;
			}
			else
				iter_file(files[i], data, &file_info[i - 1]);

		}
		sleep(15);
	}
	printf("entropy process is stopped\n");
	return (NULL);
}
