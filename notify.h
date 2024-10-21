#ifndef H_NOTIFY
#define H_NOTIFY

#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdbool.h>

struct	inotify_args
{
	int	argc;
	char	**argv;
};

typedef struct	s_file_info
{
	char		*file;
	long long	baseline_mtime;
}	t_file_info;

typedef struct s_dir_tree
{
	char			*base_dir;
	char			*file;
	long long		baseline_mtime;
	struct t_dir_tree	*next;
}	t_dir_tree;

typedef struct s_dir
{
	char			base_dir[1024];
	long long		baseline_mtime;
	struct t_dir_tree	**dir_tree;
	struct s_dir		*next;
}	t_dir;

bool	check_modification_time(char *dir, long long baseline_mtime);
t_dir	**count_dirs(int argc, t_file_info *info);

#endif
