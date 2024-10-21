#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include "notify.h"

static void	add_create_dirtree(char *entry, char *dir, t_dir **dir_tree, int count)
{
	int		i;
	struct stat	st;
	char		file_path[1024];


	i = 0;
	if (strstr(entry, dir) == NULL)
		snprintf(file_path, sizeof(file_path), "%s/%s", dir, entry);
	else
		snprintf(file_path, sizeof(file_path), "%s", entry);
	while (dir_tree[i])
	{
		if (strcmp(dir_tree[i]->base_dir, dir) != 0)
			continue ;
		else
		{
			if (stat(file_path, &st) == -1)
			{
				perror("stat error");
				exit (1);
			}
			dir_tree[i]->tree[count] = (t_file_info *)malloc(sizeof(t_file_info));
			if (!dir_tree[i]->tree[count])
			{
				perror("malloc error");
				exit (1);
			}
			strncpy(dir_tree[i]->tree[count]->file_name, entry, sizeof(dir_tree[i]->tree[count]->file_name) - 1);
			dir_tree[i]->tree[count]->file_name[sizeof(dir_tree[i]->tree[count]->file_name) - 1] = '\0';
			dir_tree[i]->tree[count]->baseline_mtime = st.st_mtime;
			printf("File: %s, mtime: %lld\n", dir_tree[i]->tree[count]->file_name, dir_tree[i]->tree[count]->baseline_mtime);
			break ;
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
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
			ret += count_files(entry->d_name);
		else
			ret++;
	}
	closedir(dp);
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
			for (int j = 0; j < (int)nbr_files; j++)
				dir_tree[i]->tree[j] = NULL;
			dir_tree[i]->tree[nbr_files] = NULL;
			break ;
		}
		i++;
	}
}

static void	track_recursive(char *main_dir, char *file_path, t_dir **dir_tree, int *count)
{
	DIR		*dp;
	struct dirent	*entry;
	char		new_file_path[1024];

	if ((dp = opendir(file_path)) == NULL)
	{
		perror("opendir error");
		exit (1);
	}
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{

			snprintf(new_file_path, sizeof(new_file_path), "%s/%s", file_path, entry->d_name);
			track_recursive(main_dir, new_file_path, dir_tree, count);
		}
		else
		{
			if (DEBUG)
			{
				printf("file_path es %s\n", file_path);
				printf("entry->d_name es %s\n", entry->d_name);
			}
			snprintf(new_file_path, sizeof(new_file_path), "%s/%s", file_path, entry->d_name);
			add_create_dirtree(new_file_path, main_dir, dir_tree, *count);
			*count += 1;
		}
	}
	closedir(dp);
}

void	track_dir_mtime(char *dir, t_dir **dir_tree)
{
	DIR				*dp;
	struct dirent	*entry;
	char			file_path[1024];
	int			count;

	if ((dp = opendir(dir)) == NULL)
	{
		perror("opendir error");
		exit (1);
	}
	count = 0;
	init_mtime_main_dir(dir, dir_tree);
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{
			snprintf(file_path, sizeof(file_path), "%s/%s", dir, entry->d_name);
			track_recursive(dir, file_path, dir_tree, &count);
		}
		else
		{
			add_create_dirtree(entry->d_name, dir, dir_tree, count);
			count++;
		}
	}
	closedir(dp);
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
		dirs = (t_dir **)malloc(sizeof(t_dir *) * (count + 1));
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
