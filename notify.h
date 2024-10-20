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

bool	check_modification_time(char *dir, long long baseline_mtime);

#endif
