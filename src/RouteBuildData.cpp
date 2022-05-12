#include "RouteBuildData.h"
using namespace WeatherUtils;

bool WeatherUtils::CheckGetCachedRouteBuildData(vector<RouteBuildData>& items, BuildRouteType type, Route* route, ShipProperties& ship, int start_time_index, RouteBuildData *dst) {
	for each (auto item in items)
	{
		if (item.CheckCachedVersion(type, route, ship, start_time_index)) {
			*dst = item;
			return true;
		}
	}
	return false;
}