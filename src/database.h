#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdint.h>

struct db_ctx {
	sqlite3 * db;
	sqlite3_stmt * insert_file_stmt;
	sqlite3_stmt * delete_file_stmt;
	sqlite3_stmt * load_file_by_name_stmt;
	sqlite3_stmt * load_file_by_hash_stmt;
	int version;
};

struct db_ctx * database_init(const char * filename, char ** err_message);
void database_close(struct db_ctx * ctx);
int database_fileadd(struct db_ctx * ctx, const char * filename, int64_t modified_time, void * hash);
int database_filefindbyname(struct db_ctx * ctx, const char * filename, int64_t * modified_time_ptr, void * hash_ptr);

#endif // DATABASE_H

