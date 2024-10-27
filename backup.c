#include "notify.h"
#include <fcntl.h>
#include <signal.h>

extern void						mask_signals(void);
extern volatile sig_atomic_t	stop;
extern void						alert(const char *msg);

void	create_relative_file(char *file, char *dst_dir)
{
	int		fd;
	char	backup_file[256] = {0};
	char	buf[10000] = {0};
	char	err_msg[1024] = {0};
	int		n;

	if (DEBUG)
	{
		printf("\033[33mCreating relative file %s\033[0m\n", file);
		printf("file src %s\n", file);
		printf("dst dir %s\n", dst_dir);

	}
	if (endswith(dst_dir, ".bak"))
		snprintf(backup_file, sizeof(backup_file), "%s", dst_dir);
	else
		snprintf(backup_file, sizeof(backup_file), "%s.bak", dst_dir);
	if (DEBUG)
		printf("backup file %s\n", backup_file);
	if ((fd = open(file, O_RDONLY)) == -1)
	{
		if (DEBUG)
			printf("file %s\n", file);
		snprintf(err_msg, sizeof(err_msg), "open error: %s", file);
		perror(err_msg);
		exit (1);
	}
	if ((n = read(fd, buf, sizeof(buf))) == -1)
	{
		snprintf(err_msg, sizeof(err_msg), "read error: %s", file);
		perror(err_msg);
		exit (1);
	}
	if (close(fd) == -1)
	{
		perror("close error");
		exit (1);
	}
	if ((fd = open(backup_file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
	{
		snprintf(err_msg, sizeof(err_msg), "open 2 error: %s", backup_file);
		perror(err_msg);
		exit (1);
	}
	if (write(fd, buf, n) == -1)
	{
		snprintf(err_msg, sizeof(err_msg), "write error: %s", backup_file);
		perror(err_msg);
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

static void	check_modification(t_file_info *info, t_dir **track_dir, bool is_dir, char *dst_dir)
{
	struct stat	st;
	char		file_path[1024] = {0};
	char		msg[1024] = {0};

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
			printf("\033[33mCreating backup directory %s\033[0m\n", info->file);
			create_backup_directory(info->file, dst_dir);
			track_dir_mtime(info->file, track_dir);
			set_backup_dir(info->file, dst_dir, track_dir);
		}
		else
		{
			printf("creating backup file\n");
			if (info->file[0] != '/')
			{
				snprintf(file_path, sizeof(file_path), "%s%s.bak", dst_dir, info->file);
				create_relative_file(info->file, file_path);
			}
			else
			{
				create_backup_file(info->file, dst_dir);
			}
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

void	*backup(void *args)
{
	t_dir				**track_dir;
	int					i;
	int					argc;
	char				**argv;
	int					interval;
	struct stat			st;
	char				home_dir[1024] = {0};
	struct inotify_args	*arguments;


	mask_signals();
	arguments = (struct inotify_args *)args;
	argc = arguments->argc;
	argv = arguments->argv;
	interval = arguments->bk_interval;
	get_home_dir(home_dir, sizeof(home_dir));
	if (DEBUG)
		printf("home dir %s\n", home_dir);
	if (argc < 2)
	{
		printf("Usage: back-up [file | directory]\n");
		exit (1);
	}
	t_file_info			info[argc - 1];
	for (i = 1; i < argc; i++)
	{
		info[i - 1].file = argv[i];
		info[i - 1].baseline_mtime = -1;
	}
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
				printf("sending directory %s\n", info[i].file);
				check_modification(&info[i], track_dir, true, home_dir);
			}
			else
			{
				check_modification(&info[i], track_dir, false, home_dir);
			}
		}
		sleep(interval);
	}
	ft_free(track_dir);
	printf("\033[33mbackup stopped\033[0m\n");
	return (NULL);
}
