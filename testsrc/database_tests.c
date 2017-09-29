#include <stdio.h>
#include <stdlib.h>
#include "database.h"

int main(int argc, char* argv[]) {
	struct db_ctx * db_ctx;
	char * err_msg;

	db_ctx = database_init("testing.db", &err_msg);

	if (db_ctx == NULL) {
		fprintf(stdout, "Could not initalise database context.\n%s\n", err_msg);
		return 1;
	}

	fprintf(stdout, "Version: %d\n", db_ctx->version);

	free(db_ctx);

	return 0;
}

