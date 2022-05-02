#ifndef __ROUTE_BUILD_INFO__
#define __ROUTE_BUILD_INFO__

#include <string>
#include <vector>
#include "Route.h"
#include "ship_properties.h"
#include "weather_render_utilities.h"
#include "weather_utils.h"

using namespace std;

namespace WeatherUtils
{
	class RouteBuildData {
	private:
		double lat_min;
		double lon_min;

		///cache key
		/*RoutePoint* start;
		RoutePoint* finish;*/
		Route* route;
		ShipProperties ship;
		int start_time_index;
	public:
		/// cache data
		vector<vector<int>> considered_zone_grid;
		vector<pair<double, pair<int, int>>> optimal_path;

		RouteBuildData(double lat_min, double lon_min) : lat_min(lat_min), lon_min(lon_min) {}
		RouteBuildData(double lat_min, double lon_min, Route* route, ShipProperties &ship, int start_time_index) :
			lat_min(lat_min), lon_min(lon_min), ship(ship), start_time_index(start_time_index)
		{
			/*this->start = CopyRoutePoint(start);
			this->finish = CopyRoutePoint(finish);*/
			this->route = new Route();
			CloneRoute(this->route, route, 1, route->GetnPoints(), "cloned");
		}
		virtual ~RouteBuildData() 
		{
			considered_zone_grid.clear();
			optimal_path.clear();
		};
		
		bool CheckCachedVersion(Route* route, ShipProperties& ship, int start_time_index) {
			return this->route->IsEqualTo(route) &&
				this->ship == ship &&
				this->start_time_index == start_time_index;
		}

		void Render(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
			auto orange = wxColour(255, 165, 0, 255);
			
			highlight_considered_grid(considered_zone_grid, cc, dc, VP, box);
			
			for (int i = 0; i < optimal_path.size(); i++) {
				print_zone(cc, dc, VP, box, 
					lat_min + ((double)optimal_path[i].second.first) / 10,
					lon_min + ((double)optimal_path[i].second.second) / 10,
					orange);
			}
		}
	private:
		void highlight_considered_grid(std::vector<std::vector<int>> &grid, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
			auto darkRed = wxColour(200, 0, 0, 255);

			for (int i = 0; i < grid.size(); i++) {
				for (int j = 0; j < grid[0].size(); j++) {
					double lat = get_coordinate_from_index(i, lat_min);
					double lon = get_coordinate_from_index(j, lon_min);
					if (grid[i][j] == 0 || grid[i][j] == 1) {
						WeatherUtils::print_zone(cc, dc, VP, box, lat, lon, darkRed);
					}
				}
			}
		}

		RoutePoint* CopyRoutePoint(RoutePoint* src) {
			return new RoutePoint(src->GetLatitude(), src->GetLongitude(), src->GetIconName(), src->GetName(), wxEmptyString, false);
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

	bool CheckGetCachedRouteBuildData(vector<RouteBuildData>& items, Route* route, ShipProperties& ship, int start_time_index, RouteBuildData *dst);
}

#endif