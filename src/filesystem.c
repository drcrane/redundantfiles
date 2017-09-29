#define _DEFAULT_SOURCE

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void filesystem_recursedirectories(char * name) {
	DIR * dir;
	struct dirent * entry;
	struct stat status;
	int rc;

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
			rc = lstat(path, &status);
			if (rc == -1) {
				continue;
			}
			// not following directory symbolic links for now.
			if (S_ISLNK(status.st_mode)) {
				continue;
			}
			filesystem_recursedirectories(path);
		} else {
			fprintf(stdout, "%s - %s\n", name, entry->d_name);
		}
	}
	closedir(dir);
}

