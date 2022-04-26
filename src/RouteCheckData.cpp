#include "RouteCheckData.h"

using namespace WeatherUtils;

void WeatherUtils::RemoveStaleCachedRouteCheckData(vector<RouteCheckData>& items, RouteCheckData& data)
{
	int removeIndex = 0;
	for (removeIndex; removeIndex < items.size(); removeIndex++)
	{
		auto item = items[removeIndex];
		if (item.CheckStaleCachedVersion(data)) {
			break;
		}
	}
	items.erase(items.begin() + removeIndex);
	return;
}

bool WeatherUtils::CheckGetCachedRouteCheckData(vector<RouteCheckData>& items, Route* route, ShipProperties& ship, int startTimeIndex, RouteCheckData *dst) {
	for each (auto item in items)
	{
		if (item.CheckCachedVersion(route, ship, startTimeIndex)) {
			*dst = item;
			return true;
		}
	}
	return false;
}
