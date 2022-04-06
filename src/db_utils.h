#ifndef __DBUTILS__
#define __DBUTILS__

#include <string>
#include <vector>
#include "refuge_place.h"
#include "ship_class.h"

namespace DbUtils 
{
constexpr auto DEFAULT_DB_FILE_PATH = "C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\weather_db.db";

class DbContext {
	const char* SQL_QUERY_EXAMPLE = "CREATE TABLE IF NOT EXISTS foo(a,b,c); INSERT INTO FOO VALUES(1,2,3);";
	const std::string DefaultFilePath = DEFAULT_DB_FILE_PATH;
	private:
		std::string m_db_file_path;
	
	public:
		DbContext();
		DbContext(std::string file_name);
		virtual ~DbContext();

		int SeedDefaultData();
		std::vector<WeatherUtils::RefugePlace> QuerySafePlaceData();
		std::vector<WeatherUtils::ShipClass> QueryShipClasses();
		int GetWaveHeightByShipClass(std::string ship_class);
	private:
		//int callback(void *data, int argc, char **argv, char **azColName);
	};
}

#endif