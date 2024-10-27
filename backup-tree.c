#include "notify.h"

static void	set_backup_path(char *file_name, char *backup_path)
{
		int	i;
		int	j;

		i = 0;
		j = 0;
		for (j = 0; backup_path[j] != '\0'; j++)
				;
		while (file_name[i] != '\0')
		{
			if (file_name[i] == '/' && i != 0)
			{
				memcpy(backup_path + j, ".bak", 4);
				j += 4;
			}
			backup_path[j] = file_name[i];
			i++;
			j++;
		}
		memcpy(backup_path + j, ".bak", 4);
		j += 4;
		backup_path[j] = '\0';
}

static char	*get_my_home(char *home_dir, size_t len)
{
	const char		*sudo_user;
	struct passwd	*pwd;
	uid_t			uid;
	size_t			len_tmp;

	if (home_dir == NULL || len == 0)
	{
		fprintf(stderr, "Error: invalid arguments\n");
		exit(1);
	}
	memset(home_dir, 0, len);
	sudo_user = getenv("SUDO_USER");
	if (sudo_user != NULL)
			pwd = getpwnam(sudo_user);
	else
	{
		uid = getuid();
		pwd = getpwuid(uid);
	}
	if (pwd == NULL)
	{
		fprintf(stderr, "Error: getpwuid failed\n");
		exit(1);
	}
	len_tmp = strlen(pwd->pw_dir);
	if (len_tmp + 2 > len)
	{
		fprintf(stderr, "Error: home directory too long\n");
		exit(1);
	}
	strncpy(home_dir, pwd->pw_dir, len - 1);
	home_dir[len - 1] = '\0';
	strncat(home_dir, "/", len - strlen(home_dir) - 1);
	return (home_dir);
}

static const char	*get_last_occurence(const char *str, const char *prefix)
{
	size_t	len;
	char	*tmp;
	size_t	len_tmp;
	char	home_dir[1024] = {0};

	get_my_home(home_dir, 1024);
	if (strstr(str, home_dir) == NULL)
		return (str);
	tmp = strstr(prefix, "backup/");
	len_tmp = strlen(tmp);
	len = strlen(prefix);
	len = len - len_tmp;
	if (strncmp(str, prefix, len) == 0)
			return (str + len);
	return (prefix);
}

void	set_backup_dir(char *main_dir, char *backup_dir, t_dir **track_dir)
{
	int			i;
	int			j;
	const char	*tmp_file;

	i = 0;
	while (track_dir[i] != NULL)
	{
		if (strcmp(track_dir[i]->base_dir, main_dir) == 0)
		{
			j = 0;
			while (track_dir[i]->tree[j] != NULL)
			{
				memset(track_dir[i]->tree[j]->backup_path, 0, 1024);
				tmp_file = get_last_occurence(track_dir[i]->tree[j]->file_name, backup_dir);
				if (tmp_file == NULL)
				{
						fprintf(stderr, "Error: file not found\n");
						exit(1);
				}
				strncpy(track_dir[i]->tree[j]->backup_path, backup_dir, strlen(backup_dir));
				set_backup_path((char *)tmp_file, track_dir[i]->tree[j]->backup_path);
				j++;
			}
		}
		i++;
	}
}
