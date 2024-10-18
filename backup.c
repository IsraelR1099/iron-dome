#include "notify.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

static void	check_modification(int argc, struct file_info *info)
{
	struct stat	st;

	if (stat(info->file, &st) == -1)
	{
		perror("stat error");
		exit (1);
	}
	if (file->baseline_mtime == -1)
		file->baseline_mtime = st.st_mtime;
	printf("modification time %ld\n", st.st_mtime);
	printf("info file %s\n", info->file);
	(void)argc;
}



int	main(int argc, char **argv)
{
	struct file_info	info[argc - 1];
	int			i;
	struct stat		st;

	if (argc < 2)
	{
		printf("Usage: back-up [file | directory]\n");
		exit (1);
	}
	for (i = 1; i < argc; i++)
	{
		info[i - 1].file = argv[i];
		info[i - 1].baseline_mtime = -1;
	}
	for (i = 0; i < argc - 1; i++)
		printf("info %s\n", info[i].file);
	for (i = 0; i < argc - 1; i++)
	{
		if (stat(info[i].file, &st) == -1)
		{
			perror("stat error");
			exit (1);
		}
		if (S_ISDIR(st.st_mode))
		{
			printf("%s is a directory\n", info[i].file);
			check_modification(argc, &info[i]);
		}
		else
		{
			printf("%s is a file\n", info[i].file);
			check_modification(argc, &info[i]);
		}
	}
	return (0);
}
