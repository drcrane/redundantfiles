#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "database.h"

static int database_create_schema(sqlite3 * db);

/*
static char * my_strdup(char * str) {
	char * dup;
	size_t len;
	len = strlen(str);
	len ++;
	dup = malloc(len);
	if (dup == NULL) {
		return NULL;
	}
	memcpy(dup, str, len);
	return dup;
}
*/

static int database_init_callback(struct db_ctx * db_ctx, int col_count, 
		char ** col_values, char ** col_names) {
	int i;

	// if this is called again that means two rows!
	// we do not support two rows.
	if (db_ctx->version != 0) {
		return 1;
	}

	for (i = 0; i < col_count; i ++) {
		//fprintf(stdout, "%d: %s = %s\n", i, col_names[i], col_values[i]);
		if (col_values[i] != NULL && strcmp(col_names[i], "version") == 0) {
			db_ctx->version = atoi(col_values[i]);
			break;
		}
	}
	return 0;
}

struct db_ctx * database_init(const char * filename, char ** err_message) {
	sqlite3 *db;
	struct db_ctx * db_ctx = NULL;
	char * errmsg = NULL;
	int rc;

	rc = sqlite3_open(filename, &db);
	if (rc) {
		*err_message = (char *)sqlite3_errmsg(db);
		sqlite3_close(db);
		return NULL;
	}
	db_ctx = malloc(sizeof(struct db_ctx));
	db_ctx->db = db;
	db_ctx->version = 0;
	rc = sqlite3_exec(db, "SELECT * FROM filedb_version_meta ;", 
		(int (*)(void *, int,  char **, char **))database_init_callback, db_ctx, &errmsg);
	if (rc == SQLITE_ABORT) {
		*err_message = "The filedb_version_meta table has more than one row.";
		goto error;
	}
	if (rc != SQLITE_OK) {
		if (database_create_schema(db)) {
			*err_message = "Schema creation problem";
			goto error;
		}
		db_ctx->version = 1;
	} else {
		if (db_ctx->version != 1) {
			*err_message = "Wrong version in database file.";
			goto error;
		}
	}
	return db_ctx;
error:
	if (db_ctx != NULL) {
		free(db_ctx);
		db_ctx = NULL;
	}
	return db_ctx;
}

static int database_create_schema(sqlite3 * db) {
	int rc;
	char * errmsg = NULL;
	rc = sqlite3_exec(db, "CREATE TABLE filedb_version_meta (version INTEGER, banner TEXT) ;", NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		goto error;
	}
	rc = sqlite3_exec(db, "INSERT INTO filedb_version_meta (version, banner) VALUES (1, 'PRE-RELEASE-VERSION') ;", NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		goto error;
	}
	rc = sqlite3_exec(db, "CREATE TABLE filedb_file (filename TEXT, modifiedtime INTEGER, hash BLOB) ;", NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		goto error;
	}
	return 0;
error:
	if (errmsg != NULL) {
		sqlite3_free(errmsg);
	}
	return -1;
}

