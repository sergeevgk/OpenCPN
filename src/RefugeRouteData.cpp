#include "RefugeRouteData.h"
using namespace WeatherUtils;

bool WeatherUtils::CheckGetCachedRouteRefugeData(vector<RefugeRouteData>& items, SimpleRoutePoint* conflict, SimpleRoutePoint* refuge, ShipProperties& ship, int start_time_index, RefugeRouteData * dst)
{
	for each (auto item in items)
	{
		if (item.CheckCachedVersion(conflict, refuge, ship, start_time_index)) {
			*dst = item;
			return true;
		}
	}
	return false;
}
