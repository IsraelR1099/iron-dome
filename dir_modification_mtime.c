#include "notify.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

static void	create_new_backup(char *file, char *backup_path)
{
	int		fd;
	char	back_file[256];
	char	buf[10000];
	char	error_msg[1024];
	int		n;
	char	home_dir[128];

	get_home_dir(home_dir, sizeof(home_dir));
	snprintf(back_file, sizeof(back_file), "%s%s.bak", home_dir, backup_path);
	if ((fd = open(file, O_RDONLY)) < 0)
	{
		snprintf(error_msg, sizeof(error_msg), "Error open: %s", file);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	if ((n = read(fd, buf, sizeof(buf))) < 0)
	{
		snprintf(error_msg, sizeof(error_msg), "Error read: %s", file);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	close(fd);
	if ((fd = open(back_file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
	{
		snprintf(error_msg, sizeof(error_msg), "Error open: %s", back_file);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	if (write(fd, buf, n) != n)
	{
		snprintf(error_msg, sizeof(error_msg), "Error write: %s", back_file);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	close(fd);
	printf("backup file %s created\n", back_file);
}

static bool	check_file_mtime(char *file, char *main_dir, t_dir **track_dir)
{
	int			i;
	int			j;
	struct stat	st;
	char		error_msg[1024];

	i = 0;
	while (track_dir[i] != NULL)
	{
		if (strcmp(track_dir[i]->base_dir, main_dir) == 0)
		{
			j = 0;
			while (track_dir[i]->tree[j] != NULL)
			{
				if (strcmp(track_dir[i]->tree[j]->file_name, file) == 0)
				{
					if (stat(file, &st) < 0)
					{
						snprintf(error_msg, sizeof(error_msg), "Error stat: %s", file);
						perror(error_msg);
						exit(EXIT_FAILURE);
					}
					if (DEBUG)
					{
						printf("st.st_mtime: %ld, baseline_mtime: %lld\n", st.st_mtime, track_dir[i]->tree[j]->baseline_mtime);
					}
					if (st.st_mtime > track_dir[i]->tree[j]->baseline_mtime)
					{
						printf("file %s has been modified\n", file);
						track_dir[i]->tree[j]->baseline_mtime = st.st_mtime;
						create_new_backup(file, track_dir[i]->tree[j]->backup_path);
						return (true);
					}
				}
				j++;
			}
		}
		i++;
	}
	return (false);
}

static bool	check_subdir_mtime(char *dir, char *main_dir, t_dir **track_dir)
{
	DIR				*dp;
	struct dirent	*entry;
	char			error_msg[1024];
	char			full_path[1024];
	bool			modified;

	modified = false;
	if ((dp = opendir(dir)) == NULL)
	{
		snprintf(error_msg, sizeof(error_msg), "Error opendir: %s", dir);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
			|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{
			snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);
			if (check_subdir_mtime(full_path, main_dir, track_dir) == true)
			{
				modified = true;
				break ;
			}
		}
		else
		{
			snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);
			if (check_file_mtime(full_path, main_dir, track_dir) == true)
			{
				modified = true;
				break ;
			}
		}
	}
	if (closedir(dp) < 0)
	{
		snprintf(error_msg, sizeof(error_msg), "Error closedir: %s", dir);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	return (modified);
}

/*
 * Check if any file of main_dir (argv[n] passed) has been modified
 * If so, update the baseline_mtime of the file.
 * track_dir is a list of all files inside that directory (track_dir->tree)
 * Return true if any file has been modified
 */

bool	check_dir_mtime(char *main_dir, t_dir **track_dir)
{
	DIR				*dp;
	struct dirent	*entry;
	char			error_msg[1024];
	char			full_path[1024];
	bool			modified;

	modified = false;
	if ((dp = opendir(main_dir)) == NULL)
	{
		snprintf(error_msg, sizeof(error_msg), "Error opendir: %s", main_dir);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
			|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{
			snprintf(full_path, sizeof(full_path), "%s/%s", main_dir, entry->d_name);
			if (check_subdir_mtime(full_path, main_dir, track_dir) == true)
			{
				modified = true;
				break ;
			}
		}
		else
		{
			snprintf(full_path, sizeof(full_path), "%s/%s", main_dir, entry->d_name);
			if (check_file_mtime(full_path, main_dir, track_dir) == true)
			{
				modified = true;
				break ;
			}
		}
	}
	if (closedir(dp) < 0)
	{
		snprintf(error_msg, sizeof(error_msg), "Error closedir: %s", main_dir);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	return (modified);
}
