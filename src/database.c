#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "database.h"

#include "dbg.h"

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
	sqlite3 * db = NULL;
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
	memset(db_ctx, 0, sizeof(struct db_ctx));
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
	rc = sqlite3_prepare_v2(db_ctx->db, "INSERT INTO filedb_file (filename, modifiedtime, hash) VALUES (?, ?, ?) ;", -1, &db_ctx->insert_file_stmt, NULL);
	if (rc != SQLITE_OK) { goto query_prep_error; }
	rc = sqlite3_prepare_v2(db_ctx->db, "SELECT modifiedtime, hash FROM filedb_file WHERE filename = ? ;", -1, &db_ctx->load_file_by_name_stmt, NULL);
	if (rc != SQLITE_OK) { goto query_prep_error; }
	return db_ctx;
query_prep_error:
	*err_message = "database query preparation error";
error:
	if (db_ctx != NULL) {
		database_close(db_ctx);
	}
	return NULL;
}

void database_close(struct db_ctx * ctx) {
	if (ctx->db) {
		if (ctx->insert_file_stmt) {
			sqlite3_finalize(ctx->insert_file_stmt);
		}
		sqlite3_close(ctx->db);
	}
	free(ctx);
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
	rc = sqlite3_exec(db, "CREATE TABLE filedb_file (filename TEXT, modifiedtime INTEGER, hash BLOB, PRIMARY KEY (filename) ) ;", NULL, NULL, &errmsg);
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

int database_fileadd(struct db_ctx * ctx, const char * filename, int64_t modified_time, void * hash) {
	int rc;
	int res;
	res = -1;
	rc = sqlite3_bind_text(ctx->insert_file_stmt, 1, filename, -1, SQLITE_STATIC);
	if (rc != SQLITE_OK) { goto error; }
	rc = sqlite3_bind_int64(ctx->insert_file_stmt, 2, modified_time);
	if (rc != SQLITE_OK) { goto error; }
	rc = sqlite3_bind_blob(ctx->insert_file_stmt, 3, hash, 64, SQLITE_STATIC);
	if (rc != SQLITE_OK) { goto error; }
	rc = sqlite3_step(ctx->insert_file_stmt);
	if (rc == SQLITE_CONSTRAINT) {
		// This file must already exist
		res = 1;
	} else
	if (rc == SQLITE_DONE) {
		// The record was appended
		res = 0;
	} else
	{ goto error; }
error:
	// this may return an error code but we have already dealt with it
	// through the step call above so at this point we dont care.
	sqlite3_reset(ctx->insert_file_stmt);
	return res;
}

int database_filefindbyname(struct db_ctx * ctx, const char * filename, int64_t * modified_time_ptr, void * hash_ptr) {
	int rc;
	int res = -1;
	rc = sqlite3_bind_text(ctx->load_file_by_name_stmt, 1, filename, -1, SQLITE_STATIC);
	if (rc != SQLITE_OK) { goto error; }
	rc = sqlite3_step(ctx->load_file_by_name_stmt);
	//debug("rc: %d", rc);
	if (rc == SQLITE_ROW) {
		int len;
		void * ptr;
		*modified_time_ptr = sqlite3_column_int64(ctx->load_file_by_name_stmt, 0);
		ptr = (void *)sqlite3_column_blob(ctx->load_file_by_name_stmt, 1);
		len = sqlite3_column_bytes(ctx->load_file_by_name_stmt, 1);
		if (len != 64) {
			goto error;
		}
		memcpy(hash_ptr, ptr, len);
		res = 0;
	} else
	if (rc == SQLITE_DONE) {
		res = 1;
	}
error:
	sqlite3_reset(ctx->load_file_by_name_stmt);
	return res;
}

