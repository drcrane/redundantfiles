#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LINUX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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
#endif

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
/*
 * StackOverflow:
 * https://stackoverflow.com/questions/2038912/how-to-recursively-traverse-directories-in-c-on-windows
*/

static void FindFilesRecursively(LPCTSTR lpFolder, LPCTSTR lpFilePattern) {
	TCHAR szFullPattern[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile;
	// first we are going to process any subdirectories
	PathCombine(szFullPattern, lpFolder, _T("*"));
	hFindFile = FindFirstFile(szFullPattern, &FindFileData);
	if(hFindFile != INVALID_HANDLE_VALUE) {
		do {
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// found a subdirectory; recurse into it
				PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
				FindFilesRecursively(szFullPattern, lpFilePattern);
			}
		} while(FindNextFile(hFindFile, &FindFileData));
		FindClose(hFindFile);
	}

	// Now we are going to look for the matching files
	PathCombine(szFullPattern, lpFolder, lpFilePattern);
	hFindFile = FindFirstFile(szFullPattern, &FindFileData);
	if(hFindFile != INVALID_HANDLE_VALUE) {
		do {
			if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				// found a file; do something with it
				PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);
				_tprintf_s(_T("%s\n"), szFullPattern);
			}
		} while(FindNextFile(hFindFile, &FindFileData));
		FindClose(hFindFile);
	}
}


void filesystem_recursedirectories(char * name) {
	HANDLE directory;
}
#endif
