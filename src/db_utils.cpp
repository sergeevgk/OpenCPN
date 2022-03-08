#include "db_utils.h"
using namespace DbUtils;


DbContext::DbContext()
{
}

DbContext::DbContext(std::string file_path) {

}

DbUtils::DbContext::~DbContext()
{
}

int DbContext::SeedDefaultData() {
	sqlite3 *db = 0; // хэндл объекта соединение к БД
	char *err = 0;
	if (sqlite3_open(m_db_file_path.c_str(), &db))
		fprintf(stderr, "Ошибка открытия/создания БД: %s\n", sqlite3_errmsg(db));
	// выполняем SQL
	else if (sqlite3_exec(db, SQL_QUERY_EXAMPLE, 0, 0, &err))
	{
		fprintf(stderr, "Ошибка SQL: %sn", err);
		sqlite3_free(err);
	}
	// закрываем соединение
	sqlite3_close(db);
	return 0;
}