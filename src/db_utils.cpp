#include <sqlite3.h>
#include "db_utils.h"
using namespace DbUtils;
using namespace WeatherUtils;

DbContext::DbContext()
{
	m_db_file_path = std::string(DEFAULT_DB_FILE_PATH);
}

DbContext::DbContext(std::string file_path) : m_db_file_path(file_path) {

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

static int callback(void *data, int argc, char **argv, char **azColName) {
	auto p_vector_data = ((std::vector<WeatherUtils::RefugePlace> *)data);
	int id = std::stoi(argv[0]);
	std::string name = argv[1];
	double lat = std::strtod(argv[2], nullptr);
	double lon = std::strtod(argv[3], nullptr);
	int ship_class = std::stoi(argv[4]);
	RefugePlace p = RefugePlace(id, name, lat, lon, ship_class);
	p_vector_data->push_back(p);
	return 0;
}

std::vector<WeatherUtils::RefugePlace> DbUtils::DbContext::QuerySafePlaceData()
{
	std::vector<WeatherUtils::RefugePlace> result = std::vector<WeatherUtils::RefugePlace>();
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql;
	const char* data = "Callback function called";

	/* Open database */
	rc = sqlite3_open(m_db_file_path.c_str(), &db);

	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return result;
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

	/* Create SQL statement */
	sql = "SELECT * from REFUGE_PLACES";

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, callback, (void*)(&result), &zErrMsg);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return result;
}
