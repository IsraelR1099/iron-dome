#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

void monitor_inotify(int fd)
{
    char buf[BUF_LEN] __attribute__((aligned(__alignof__(struct inotify_event))));
    int len, i;

    // Loop to continually read events
	while (1)
	{
		len = read(fd, buf, BUF_LEN);
		if (len < 0)
		{
			perror("read");
			return;
		}
		i = 0;
		while (i < len)
		{
			struct inotify_event *event = (struct inotify_event *) &buf[i];
			// Check if the event has a name (for file/dir)
			if (event->len)
			{
				if (event->mask & IN_MODIFY)
				{
					if (event->mask & IN_ISDIR)
						printf("The directory %s was modified.\n", event->name);
					else
						printf("The file %s was modified.\n", event->name);
				}
				if (event->mask & IN_ACCESS)
				{
					if (event->mask & IN_ISDIR)
						printf("The directory %s was accessed.\n", event->name);
					else
						printf("The file %s was accessed.\n", event->name);
				}
			}
			i += EVENT_SIZE + event->len;
		}
	}
}

int notify(char *files, char *folders)
{
	int	fd;
	int	wd_file;

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return -1;
    }

    // Add watch for files
    wd_file = inotify_add_watch(fd, files, IN_MODIFY | IN_ACCESS);
    if (wd_file < 0) {
        perror("inotify_add_watch file");
        return -1;
    }

/*    // Add watch for folders
    wd_folder = inotify_add_watch(fd, folders, IN_MODIFY | IN_ACCESS);
    if (wd_folder < 0) {
        perror("inotify_add_watch folder");
        return -1;
    }*/

    printf("Monitoring %s and %s for changes...\n", files, folders);

    // Start monitoring
    monitor_inotify(fd);

    // Clean up
    inotify_rm_watch(fd, wd_file);
//    inotify_rm_watch(fd, wd_folder);
    close(fd);

    return 0;
}
