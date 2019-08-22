#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

struct db_ctx {
	sqlite3 * db;
	int version;
};

struct db_ctx * database_init(const char * filename, char ** err_message);

#endif // DATABASE_H

