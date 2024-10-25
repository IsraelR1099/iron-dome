CC := gcc
CFLAGS := -Wall -Wextra -fPIC
LDFLAGS := -shared -lpthread

# Source files
SRCS := alert.c check_crypto.c cpu_usage.c signals.c notify.c entropy.c \
		backup.c backup-utils.c backup-tree.c free.c dir_modification_mtime.c
OBJS := $(SRCS:.c=.o)

# Output shared library
LIBRARY = clibnotify.so

# Log file location
LOG_FILE := /var/log/irondome.log

all: $(LIBRARY) log
	chmod +x iron-dome.py
	chmod +x test_crypto.py
	chmod +x test-suite.sh

$(LIBRARY): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) -lm -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

entropy.o: entropy.c
	$(CC) $(CFLAGS) -c entropy.c -o entropy.o -lm

log:
	@if [ ! -f $(LOG_FILE) ]; then \
		echo "Creating log file at $(LOG_FILE)"; \
		sudo touch $(LOG_FILE); \
		sudo chmod 666 $(LOG_FILE); \
	else \
		echo "Log file already exists at $(LOG_FILE)"; \
	fi

clean:
	rm -f $(OBJS) $(LIBRARY)

clean_log:
	rm -f $(LOG_FILE)

re: clean all

.PHONY: all clean clean_log log
