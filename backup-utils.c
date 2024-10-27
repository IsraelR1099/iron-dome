#include "notify.h"

static void	add_create_dirtree(char *entry, char *dir, t_dir **dir_tree, int count)
{
	int			i;
	struct stat	st;
	char		file_path[1024] = {0};

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
			memset(dir_tree[i]->tree[count], 0, sizeof(t_file_info));
			strncpy(dir_tree[i]->tree[count]->file_name, entry, sizeof(dir_tree[i]->tree[count]->file_name) - 1);
			dir_tree[i]->tree[count]->file_name[sizeof(dir_tree[i]->tree[count]->file_name) - 1] = '\0';
			dir_tree[i]->tree[count]->baseline_mtime = st.st_mtime;
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
	char			error_msg[1024] = {0};
	char			file_path[1024] = {0};

	ret = 0;
	if ((dp = opendir(dir)) == NULL)
	{
		snprintf(error_msg, sizeof(error_msg), "Error opening directory '%s'", dir);
		perror(error_msg);
		exit (1);
	}
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
				|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{
			snprintf(file_path, sizeof(file_path), "%s/%s", dir, entry->d_name);
			ret += count_files(file_path);
		}
		else
			ret++;
	}
	if (closedir(dp) == -1)
	{
		snprintf(error_msg, sizeof(error_msg), "Error closing directory '%s'", dir);
		perror(error_msg);
		exit (1);
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
	DIR				*dp;
	struct dirent	*entry;
	char			new_file_path[1024] = {0};

	if ((dp = opendir(file_path)) == NULL)
	{
		printf("file_path es %s\n", file_path);
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
			snprintf(new_file_path, sizeof(new_file_path), "%s/%s", file_path, entry->d_name);
			add_create_dirtree(new_file_path, main_dir, dir_tree, *count);
			*count += 1;
		}
	}
	if (closedir(dp) == -1)
	{
		perror("closedir error");
		exit (1);
	}
}

void	track_dir_mtime(char *dir, t_dir **dir_tree)
{
	DIR				*dp;
	struct dirent	*entry;
	char			file_path[1024] = {0};
	int				count;

	if ((dp = opendir(dir)) == NULL)
	{
		printf("dir es %s\n", dir);
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
			if (endswith(dir, "/"))
				snprintf(file_path, sizeof(file_path), "%s%s", dir, entry->d_name);
			else
				snprintf(file_path, sizeof(file_path), "%s/%s", dir, entry->d_name);
			if (DEBUG)
			track_recursive(dir, file_path, dir_tree, &count);
		}
		else
		{
			snprintf(file_path, sizeof(file_path), "%s/%s", dir, entry->d_name);
			add_create_dirtree(file_path, dir, dir_tree, count);
			count++;
		}
	}
	if (closedir(dp) == -1)
	{
		perror("closedir error");
		exit (1);
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
				memset(dirs[dir_index], 0, sizeof(t_dir));
				strncpy(dirs[dir_index]->base_dir, info[i].file, sizeof(dirs[dir_index]->base_dir) -1);
				dirs[dir_index]->base_dir[sizeof(dirs[dir_index]->base_dir) - 1] = '\0';
				dirs[dir_index]->baseline_mtime = -1;
				dir_index++;
				j++;
			}
		}
		dirs[j] = NULL;
	}
	return (dirs);
}

char	*get_home_dir(char *home_dir, size_t len)
{
	const char		*sudo_user;
	struct passwd	*pwd;
	uid_t			uid;
	size_t			home_dir_len;

	memset(home_dir, 0, len);
	sudo_user = getenv("SUDO_USER");
	if (sudo_user != NULL)
	{
		pwd = getpwnam(sudo_user);
	}
	else
	{
		uid = getuid();
		pwd = getpwuid(uid);
	}
	if (pwd == NULL)
	{
		perror("getpwuid error");
		exit (1);
	}
	home_dir_len = strlen(pwd->pw_dir);
	if (home_dir_len + strlen("/backup/") < len)
	{
		strncpy(home_dir, pwd->pw_dir, len - 1);
		home_dir[len - 1] = '\0';
		strncat(home_dir, "/backup/", len - home_dir_len - 1);
	}
	else
			printf("Error: home directory path too long\n");
	return (home_dir);
}
