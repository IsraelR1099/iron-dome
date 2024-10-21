#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include "notify.h"

/*bool	check_modification_time(char *dir, long long int time)
{
	printf("dir en check modification %s\n", dir);
	printf("time %lld\n", time);
	return (false);
}
*/
static void	add_create_dirtree(char *entry, char *dir, t_dir_tree **dir_tree)
{
	int				i;
	int				j;
	unsigned int	size;

	i = 0;
	j = 0;
	while (dir_tree[i])
	{
		if (strcmp(dir_tree[i]->base_dir, dir) != 0)
			continue ;
		while (dir_tree[i]->tree[j])
		{
			size = sizeof(dir_tree[i]->tree[j]->file_name);
			strncpy(dir_tree[i]->tree[j]->file_name, entry, size - 1);
			dir_tree[i]->tree[j]->file_name[size] = '\0';
			j++;
		}
		i++;
	}
}

static size_t	count_files(char *dir)
{
	DIR				*dp;
	struct dirent	*entry;
	size_t			ret;

	ret = 0;
	if ((dp = opendir(dir)) == NULL)
	{
		perror("opendir error");
		exit (1);
	}
	printf("dir es %s\n", dir);
	while ((entry = readdir(dp)) != NULL)
	{
		printf("entry es %s\n", entry->d_name);
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
			ret += count_files(entry->d_name);
		else
			ret++;
	}
	return (ret);
}

static void	init_mtime_main_dir(char *dir, t_dir **dir_tree)
{
	struct stat	st;
	size_t		nbr_files;
	int			i;

	i = 0;
	while (dir_tree[i])
	{
		if (strcmp(dir_tree[i]->base_dir, dir) == 0)
		{
			nbr_files = count_files(dir);
			printf("nbr_files es %ld", nbr_files);
			if (stat(dir_tree[i]->base_dir, &st) == -1)
			{
				perror("stat error");
				exit (1);
			}
			dir_tree[i]->baseline_mtime = st.st_mtime;
			dir_tree[i]->tree = (t_file_info **)malloc(sizeof(t_file_info *) * (nbr_files + 1));
			if (!dir_tree[i]->tree)
			{
				perror("malloc error");
				exit (1);
			}
			for (int j = 0; j < nbr_files; j++)
				dir_tree[i]->tree[j] = NULL;
			dir_tree[i]->tree[nbr_files] = NULL;
			break ;
		}
		i++;
	}
}

void	track_dir_mtime(char *dir, t_dir **dir_tree)
{
	DIR				*dp;
	struct dirent	*entry;
	char			file_path[1024];

	if ((dp = opendir(dir)) == NULL)
	{
		perror("opendir error");
		exit (1);
	}
	init_mtime_main_dir(dir, dir_tree);
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{
			snprintf(file_path, sizeof(file_path), "%s/%s", dir, entry->d_name);
			printf("dir found in track_dir_mtime\n");
		}
		else
		{
			add_create_dirtree(entry->d_name, dir, dir_tree);
		}
	}
}

t_dir	**count_dirs(int argc, t_file_info *info)
{
	struct stat	st;
	int			count;
	int			dir_index;
	int			j;
	t_dir		**dirs = NULL;

	count = 0;
	dir_index = 0;
	j = 0;
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
	if (count != 0)
	{
		dirs = (t_dir **)malloc(sizeof(t_dir *) * count);
		if (!dirs)
		{
			perror("malloc error");
			exit (1);
		}
		for (int i = 0; i < argc - 1; i++)
		{
			if (stat(info[i].file, &st) == -1)
			{
				perror("stat error");
				exit (1);
			}
			if (S_ISDIR(st.st_mode))
			{
				dirs[dir_index] = (t_dir *)malloc(sizeof(t_dir));
				if (!dirs[dir_index])
				{
					perror("malloc error");
					exit (1);
				}
				strncpy(dirs[dir_index]->base_dir, info[i].file, sizeof(dirs[dir_index]->base_dir) -1);
				dirs[dir_index]->base_dir[sizeof(dirs[dir_index]->base_dir) - 1] = '\0';
				dirs[dir_index]->baseline_mtime = -1;
				dir_index++;
				j++;
			}
		}
	}
	dirs[j] = NULL;
	return (dirs);
}
