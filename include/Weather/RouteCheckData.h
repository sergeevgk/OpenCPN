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
	struct ConeData {
		pair<double, double> startPoint;
		pair<double, double> firstEndPoint;
		pair<double, double> secondEndPoint;
		ConeData(pair<double, double> startPoint, pair<double, double> firstEndPoint, pair<double, double> secondEndPoint) :
			startPoint(startPoint), firstEndPoint(firstEndPoint), secondEndPoint(secondEndPoint) {}
	};

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
		int start_time_index;
		/// cached data
	public:
		vector<ConflictData> conflicts;
		vector<ConeData> checkCones;
		RouteCheckData()
		{
		}

		RouteCheckData(Route* route, ShipProperties& ship, int start_time_index) : ship(ship), start_time_index(start_time_index) 
		{
			this->route = new Route();
			CloneRoute(this->route, route, 1, route->GetnPoints(), "cloned");
			conflicts = vector<ConflictData>();
			checkCones = vector<ConeData>();
		}

		virtual ~RouteCheckData(){};
		bool CheckCachedVersion(Route* route, ShipProperties& ship, int start_time_index) {
			return this->route->IsEqualTo(route) &&
				this->ship == ship &&
				this->start_time_index == start_time_index;
		}
		
		bool CheckStaleCachedVersion(RouteCheckData& data) {
			return false;
		}

		/*vector<ConflictData> GetCachedConflictData() { return conflicts; }
		void SetCachedConflictData(vector<ConflictData> &data) { conflicts = data; }*/
		void Render(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
			auto red = wxColour(255, 0, 0, 255);
			for each (auto item in conflicts)
			{
				// consider type for color
				print_zone(cc, dc, VP, box, item.latitude, item.longitude);
			}
			for each (auto item in checkCones)
			{
				draw_line_on_map(cc, dc, VP, box, item.startPoint.first, item.startPoint.second, item.firstEndPoint.first, item.firstEndPoint.second, red);
				draw_line_on_map(cc, dc, VP, box, item.startPoint.first, item.startPoint.second, item.secondEndPoint.first, item.secondEndPoint.second, red);
			}
		}
	private:
		bool RoutesAreEquivalent(Route* r1, Route* r2) {
			return true;
		}

		void CloneRoute(Route *pdestroute, Route *psourceroute, int start_nPoint, int end_nPoint, const wxString & suffix)
		{
			pdestroute->m_RouteNameString = psourceroute->m_RouteNameString + suffix;
			pdestroute->m_RouteStartString = psourceroute->m_RouteStartString;
			pdestroute->m_RouteEndString = psourceroute->m_RouteEndString;
			int i;
			for (i = start_nPoint; i <= end_nPoint; i++) 
			{
				RoutePoint *psourcepoint = psourceroute->GetPoint(i);
				RoutePoint *ptargetpoint = new RoutePoint(psourcepoint->m_lat, psourcepoint->m_lon,
					psourcepoint->GetIconName(), psourcepoint->GetName(), wxEmptyString, false);

				pdestroute->AddPoint(ptargetpoint, false);
			}
		}
	};

	// TODO: needs some revision how to implement stale data checking
	void RemoveStaleCachedRouteCheckData(vector<RouteCheckData>& items, RouteCheckData& data);

	bool CheckGetCachedRouteCheckData(vector<RouteCheckData>& items, Route* route, ShipProperties& ship, int start_time_index, RouteCheckData *dst);
}

#endif