#ifndef __ROUTE_CHECK_INFO__
#define __ROUTE_CHECK_INFO__

#include <string>
#include <vector>
#include "Route.h"
#include "ship_properties.h"
#include "weather_render_utilities.h"

using namespace std;

namespace WeatherUtils
{
	class ConflictData {
	public:
		ConflictData() {};
		ConflictData(double lat, double lon, int type, double shift) : latitude(lat), longitude(lon), conflictType(type), shiftFromStartTime(shift)
		{
		};
		double latitude;
		double longitude;
		int conflictType;
		double shiftFromStartTime;
	private:
	};

	class RouteCheckData {
	private:
		/// key properties for cache cheks
		Route* route;
		ShipProperties ship;
		int startTimeIndex;
		/// cached data
	public:
		vector<ConflictData> conflicts;
		RouteCheckData()
		{
		}

		RouteCheckData(Route* route, ShipProperties& ship, int startTimeIndex) : route(route), ship(ship), startTimeIndex(startTimeIndex) 
		{
			conflicts = vector<ConflictData>();
		}

		virtual ~RouteCheckData(){};
		bool CheckCachedVersion(Route* route, ShipProperties& ship, int startTimeIndex) {
			return this->route->IsSameAs(*route) &&
				this->ship == ship &&
				this->startTimeIndex == startTimeIndex;
		}
		
		bool CheckStaleCachedVersion(RouteCheckData& data) {
			return false;
		}

		/*vector<ConflictData> GetCachedConflictData() { return conflicts; }
		void SetCachedConflictData(vector<ConflictData> &data) { conflicts = data; }*/
		void Render(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
			for each (auto item in conflicts)
			{
				// consider type for color
				print_zone(cc, dc, VP, box, item.latitude, item.longitude);
			}
		}
	};

	// TODO: needs some revision how to implement stale checking
	void RemoveStaleCachedRouteCheckData(vector<RouteCheckData>& items, RouteCheckData& data);

	bool CheckGetCachedRouteCheckData(vector<RouteCheckData>& items, Route* route, ShipProperties& ship, int startTimeIndex, RouteCheckData *dst);
}

#endif