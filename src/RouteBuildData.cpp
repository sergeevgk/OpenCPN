#include "RouteBuildData.h"
using namespace WeatherUtils;

bool WeatherUtils::CheckGetCachedRouteBuildData(vector<RouteBuildData>& items, RoutePoint* start, RoutePoint* finish, ShipProperties& ship, int start_time_index, RouteBuildData *dst) {
	for each (auto item in items)
	{
		if (item.CheckCachedVersion(start, finish, ship, start_time_index)) {
			*dst = item;
			return true;
		}
	}
	return false;
}