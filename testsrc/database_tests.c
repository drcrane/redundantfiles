#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "minunit.h"
#include "database.h"

#include <sys/types.h>
#include <unistd.h> // unlink
#include <fcntl.h> // open

struct db_ctx * db_ctx;

char * creation_test() {
	int fd;
	int res;
	fd = open("testdata/testing.db", O_RDWR);
	if (fd != -1) {
		debug("File found, deleting...");
		res = unlink("testdata/testing.db");
		mu_assert(res == 0, "cannot delete testing.db");
	}

	char * err_msg = NULL;
	db_ctx = database_init("testdata/testing.db", &err_msg);
	if (err_msg) { debug("%s", err_msg); }
	mu_assert(err_msg == NULL && db_ctx, "db_ctx should not be null");
	debug("Version: %d", db_ctx->version);
	return NULL;
}

char * attempt_to_add_duplicate_test() {
	char hash[64];
	int rc;
	memset(hash, 0, 64);
	rc = database_addfile(db_ctx, "/full/path/to/file", 0, hash);
	mu_assert(rc == 0, "should be success");
	rc = database_addfile(db_ctx, "/full/path/to/file", 1, hash);
	mu_assert(rc == 1, "should fail");
	return NULL;
}

char * all_tests() {
	mu_suite_start();
	/* although bad practice the creation test must run first. */
	/* Tests after creation should not rely on any database state that they have not set up themselves */
	mu_run_test(creation_test);
	mu_run_test(attempt_to_add_duplicate_test);
	return NULL;
}

RUN_TESTS(all_tests)

