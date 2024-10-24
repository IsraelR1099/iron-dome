#include "notify.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pwd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

//extern void						mask_signals(void);
//extern volatile sig_atomic_t	stop;
int			stop = 0;
extern void	alert(const char *msg);

static void	create_backup_file(char *file, char *dst_dir)
{
	int	fd;
	char	backup_file[256];
	char	buf[10000];
	int	n;

	if (DEBUG)
	{
		printf("\033[33mcreating backup file\033[0m\n");
		printf("file src %s\n", file);
		printf("dst dir %s\n", dst_dir);

	}
	snprintf(backup_file, sizeof(backup_file), "%s.bak", dst_dir);
	if (DEBUG)
		printf("backup file %s\n", backup_file);
	if ((fd = open(file, O_RDONLY)) == -1)
	{
		if (DEBUG)
			printf("file %s\n", file);
		perror("open error");
		exit (1);
	}
	if ((n = read(fd, buf, sizeof(buf))) == -1)
	{
		perror("read error");
		exit (1);
	}
	if (close(fd) == -1)
	{
		perror("close error");
		exit (1);
	}
	if ((fd = open(backup_file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
	{
		perror("open error");
		exit (1);
	}
	if (write(fd, buf, n) == -1)
	{
		perror("write error");
		exit (1);
	}
	if (close(fd) == -1)
	{
		perror("close error");
		exit (1);
	}
	if (DEBUG)
		printf("backup file created\n");
}

static bool	endswith(char *s, char *suffix)
{
	int	s_len;
	int	suffix_len;

	s_len = strlen(s);
	suffix_len = strlen(suffix);
	if (s_len < suffix_len)
		return (false);
	return (strcmp(s + s_len - suffix_len, suffix) == 0);
}

static void	create_dir(char *dir)
{
	char	error_msg[2048];
	char	temp[1024];
	char	*pos;

	snprintf(temp, sizeof(temp), "%s", dir);

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

static void	create_backup_directory(char *src_dir, char *backup, bool first_time)
{
	char			backup_dir[4096] = {0};
	char			tmp_dir[4096] = {0};
	char			src_path[1024];
	DIR				*dp;
	struct dirent	*entry;
	size_t			remaining_space;

	if (DEBUG)
	{
		printf("\033[33mcreating backup directory\033[0m\n");
		printf("src dir %s\n", src_dir);
		printf("backup %s\n", backup);
	}
	if (first_time)
		snprintf(backup_dir, sizeof(backup_dir), "%s%s.bak", backup, src_dir);
	else
	{
		if (endswith(backup, "/"))
			snprintf(backup_dir, sizeof(backup_dir), "%s.bak", backup);
		else
			snprintf(backup_dir, sizeof(backup_dir), "%s.bak/", backup);
	}
	create_dir(backup_dir);
	strncpy(tmp_dir, backup_dir, strlen(backup_dir));
	if ((dp = opendir(src_dir)) == NULL)
	{
		printf("src dir %s\n", src_dir);
		perror("opendir error");
		exit (1);
	}
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		remaining_space = sizeof(backup_dir) - strlen(tmp_dir) - 1;
		if (strlen(entry->d_name) < remaining_space)
		{
			if (entry->d_type == DT_DIR)
			{
				snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
				if (snprintf(backup_dir, sizeof(backup_dir), "%s", tmp_dir) >= (int)sizeof(backup_dir))
					continue;
				if (endswith(backup_dir, "/"))
					strncat(backup_dir, entry->d_name, remaining_space);
				else
				{
					strncat(backup_dir, "/", remaining_space);
					strncat(backup_dir, entry->d_name, remaining_space);
				}
				create_backup_directory(src_path, backup_dir, false);
			}
			else
			{
				if (DEBUG)
					printf("file %s\n", entry->d_name);
				snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
				if (snprintf(backup_dir, sizeof(backup_dir), "%s", tmp_dir) >= (int)sizeof(backup_dir))
					continue;
				if (endswith(backup_dir, "/"))
					strncat(backup_dir, entry->d_name, remaining_space);
				else
				{
					strncat(backup_dir, "/", remaining_space);
					strncat(backup_dir, entry->d_name, remaining_space);
				}
				create_backup_file(src_path, backup_dir);
			}
		}
		else
			fprintf(stderr, "Path is too long: skipping: %s/%s\n", tmp_dir, entry->d_name);
	}
	if (closedir(dp) == -1)
	{
		perror("closedir error");
		exit (1);
	}
	if (DEBUG)
		printf("backup directory created\n");
}

static void	check_modification(t_file_info *info, t_dir **track_dir, bool is_dir, char *dst_dir)
{
	struct stat	st;
	char		file_path[1024];
	char		msg[1024];

	if (stat(info->file, &st) == -1)
	{
		perror("stat error");
		exit (1);
	}
	if (info->baseline_mtime == -1)
	{
		info->baseline_mtime = st.st_mtime;
		if (is_dir)
		{
			create_backup_directory(info->file, dst_dir, true);
			track_dir_mtime(info->file, track_dir);
			create_backup_dir(info->file, track_dir);
		}
		else
		{
			snprintf(file_path, sizeof(file_path), "%s%s", dst_dir, info->file);
			create_backup_file(info->file, file_path);
		}
	}
	else
	{
		if (is_dir)
		{
			printf("directory checking\n");
			if (check_dir_mtime(info->file, track_dir))
			{
				printf("\033[33mdirectory %s has been modified\033[0m\n", info->file);
				snprintf(msg, sizeof(msg), "directory %s has been modified", info->file);
				//alert(msg);
			}
		}
		else
		{
			printf("file checking\n");
			printf("baseline %lld and st mtime %ld\n", info->baseline_mtime, st.st_mtime);
			if (info->baseline_mtime < st.st_mtime)
			{
				printf("\033[33mfile %s has been modified\033[0m\n", info->file);
				snprintf(msg, sizeof(msg), "file %s has been modified", info->file);
				//alert(msg);
				snprintf(file_path, sizeof(file_path), "%s%s", dst_dir, info->file);
				create_backup_file(info->file, file_path);
				info->baseline_mtime = st.st_mtime;
			}
		}
	}
}

static void	sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("received SIGINT\n");
		stop = 1;
	}
}

int	main(int argc, char **argv)
{
	t_file_info		info[argc - 1];
	t_dir			**track_dir;
	int				i;
	struct stat		st;
	char			home_dir[1024];
	signal(SIGINT, sig_handler);

	//mask_signals();
	get_home_dir(home_dir, sizeof(home_dir));
	if (DEBUG)
		printf("home dir %s\n", home_dir);
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
	if (mkdir(home_dir, 0755) == -1)
	{
		if (DEBUG)
			printf("directory '%s' already exists\n", home_dir);
		perror("mkdir error");
		exit (1);
	}
	track_dir = count_dirs(argc, info);
	while (!stop)
	{
		for (i = 0; i < argc - 1; i++)
		{
			if (stat(info[i].file, &st) == -1)
			{
				perror("stat error");
				exit (1);
			}
			if (S_ISDIR(st.st_mode))
			{
				check_modification(&info[i], track_dir, true, home_dir);
			}
			else
			{
				check_modification(&info[i], track_dir, false, home_dir);
			}
		}
		sleep(15);
	}
	ft_free(track_dir);
	printf("\033[33mbackup stopped\033[0m\n");
	return (0);
}
