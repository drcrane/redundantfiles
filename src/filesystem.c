#define _DEFAULT_SOURCE

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void filesystem_recursedirectories(char * name) {
	DIR * dir;
	struct dirent * entry;

	if (!(dir = opendir(name))) {
		return;
	}
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR) {
			char path[1024];
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}
			snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
			filesystem_recursedirectories(path);
		} else {
			fprintf(stdout, "%s - %s\n", name, entry->d_name);
		}
	}
	closedir(dir);
}

