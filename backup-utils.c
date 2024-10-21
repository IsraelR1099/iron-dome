#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "notify.h"

/*bool	check_modification_time(char *dir, long long int time)
{
	printf("dir en check modification %s\n", dir);
	printf("time %lld\n", time);
	return (false);
}

void	add_create_dirtree(char *entry, t_dir_tree **dir_tree)
{
	if (!*dir_tree)
	{

	}
}

void	track_dir_mtime(char *dir, t_dir_tree **dir_tree)
{
	DIR		*dp;
	struct dirent	*entry;
	char		file_path[1024];

	if ((dp = opendir(dir)) == NULL)
	{
		perror("opendir error");
		exit (1);
	}
	while ((entry = readdir(dp)) == NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{
			snprintf(file_path, sizeof(file_path), "%s/%s", dir, entry->d_name);
			track_dir_mtime(file_path, dir_tree);
		}
		else
		{
			add_create_dirtree(entry->d_name, dir_tree);
		}
	}
}
*/
t_dir	**count_dirs(int argc, t_file_info *info)
{
	struct stat	st;
	int		count;

	count = 0;
	for (int i = 0; i < argc - 1; i++)
	{
		if (stat(info[i].file, &st) == -1)
		{
			perror("stat error");
			exit (1);
		}
		if (S_ISDIR(st.st_mode))
			count++;
	}
	printf("count es %d\n", count);
	if (count != 0)
	{
		for (int i = 0; i <= count; i++)
		{
			if (stat(info[i].file, &st) == -1)
			{
				perror("stat error");
				exit (1);
			}
			if (S_ISDIR(st.st_mode))
		}
	}
	return (NULL);
}
