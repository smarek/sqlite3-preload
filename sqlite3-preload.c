
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Original library declarations, types and functions
*/

struct sqlite3;
typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
typedef void (*sqlite3_destructor_type)(void*);
#define SQLITE_STATIC      ((sqlite3_destructor_type)0)

static void *
lib;

static
 const char* (*orig_sqlite3_sql) (sqlite3_stmt *pStmt);

static
 char* (*orig_sqlite3_expanded_sql) (sqlite3_stmt *pStmt);

static
 int (*orig_sqlite3_bind_text) (sqlite3_stmt*,int,const char*,int,void(*)(void*));

static
int (*orig_sqlite3_prepare_v2) (
  sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
);

/*
Shim impl
*/

static void *
get_sym (
	const char *name
)
{
	const char *err;
	void *sym;

	(void)dlerror();
	sym = dlsym(lib, name);
	err = dlerror();
	if (err == 0 && sym == 0)
		err = "dlsym() returned NULL!";
	if (err != 0) {
		fprintf(stderr, "Can't find symbol %s: %s!\n", name, err);
		abort();
	}

	return sym;
}

static void
__attribute__((constructor))
load_syms (void)
{
	const char *library;
	const char *err;

	/* do not double load */
	if (lib != 0)
		return;

	/* load library */
	library = "libsqlite3.so.0";
	(void)dlerror();
	lib = dlopen(library, RTLD_NOW|RTLD_GLOBAL);
	err = dlerror();
	if (err == 0 && lib == 0)
		err = "dlopen() returned NULL!";
	if (err != 0) {
		fprintf(stderr, "Can't open library %s: %s!\n", library, err);
		abort();
	}

	/* load symbols */
	orig_sqlite3_bind_text = get_sym("sqlite3_bind_text");
	orig_sqlite3_prepare_v2 = get_sym("sqlite3_prepare_v2");
	orig_sqlite3_sql = get_sym("sqlite3_sql");
	orig_sqlite3_expanded_sql = get_sym("sqlite3_expanded_sql");
}

int sqlite3_bind_text (sqlite3_stmt* stmt,int index,const char* binded,int num_of_bytes) {
	fprintf(stdout, "bind_text:: %s\n", binded);
	int ret = orig_sqlite3_bind_text(stmt,index,binded,num_of_bytes,SQLITE_STATIC);
	fprintf(stdout,"bound:: %s\n", orig_sqlite3_expanded_sql(stmt));
	return ret;
}

int sqlite3_prepare_v2 (
sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
){
  fprintf(stdout,"prepare_v2:: %s\n", zSql);
  int ret = orig_sqlite3_prepare_v2(db,zSql,nByte,ppStmt,pzTail);
  fprintf(stdout,"prepared:: %s\n", orig_sqlite3_expanded_sql(*ppStmt));
  return ret;
}
