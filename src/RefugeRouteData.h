#ifndef __ROUTE_REFUGE_INFO__
#define __ROUTE_REFUGE_INFO__

#include <string>
#include <vector>
#include "Route.h"
#include "ship_properties.h"
#include "RouteCheckData.h"
#include "RouteBuildData.h"
#include "weather_render_utilities.h"
#include "weather_utils.h"

using namespace std;

namespace WeatherUtils
{
	static string emptyString = "";
	
	struct SimpleRoutePoint {
		double latitude;
		double longitude;
		string name;

		SimpleRoutePoint() {};
		SimpleRoutePoint(double lat, double lon, string name = emptyString) : latitude(lat), longitude(lon), name(name){};
	};

	class RefugeRouteData {
	private:
		double lat_min;
		double lon_min;
		///cache key
		SimpleRoutePoint conflict;
		SimpleRoutePoint refuge;
		ShipProperties ship;
		int start_time_index;
	public:
		enum PathType { OPTIMAL_PATH, CONFLICTS } path_type;
		/// cached data: considered grid + either optimal_path_data or check_route_data
		vector<pair<double, pair<int, int>>> optimal_path;
		vector<ConflictData> conflicts;
		vector<vector<int>> considered_zone_grid;

		RefugeRouteData() {}
		RefugeRouteData(double lat_min, double lon_min) : lat_min(lat_min), lon_min(lon_min) {}
		RefugeRouteData(double lat_min, double lon_min, SimpleRoutePoint& conflict, SimpleRoutePoint& refuge, ShipProperties& ship, int start_time_index) : 
			lat_min(lat_min), lon_min(lon_min), ship(ship), start_time_index(start_time_index)
		{
			this->conflict = SimpleRoutePoint(conflict.latitude, conflict.longitude, conflict.name);
			this->refuge = SimpleRoutePoint(refuge.latitude, refuge.longitude, refuge.name);
		}
		virtual ~RefugeRouteData() {};

		void SetOptimalPath(RouteBuildData optimalPathData) {
			this->optimal_path = optimalPathData.optimal_path;
			this->considered_zone_grid = optimalPathData.considered_zone_grid;
			this->path_type = OPTIMAL_PATH;
		}
		void SetConflictsPath(vector<ConflictData> conflicts, vector<vector<int>> considered_zone) {
			this->conflicts = conflicts;
			this->considered_zone_grid = considered_zone;
			this->path_type = CONFLICTS;
		}

		bool CheckCachedVersion(SimpleRoutePoint* conflict, SimpleRoutePoint* refuge, ShipProperties& ship, int start_time_index) {
			return IsSame(this->conflict, conflict) &&
				IsSame(this->refuge, refuge) &&
				this->ship == ship &&
				this->start_time_index == start_time_index;
		};
		void Render(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
			auto green = wxColour(0, 255, 0, 255);
			highlight_considered_grid(considered_zone_grid, cc, dc, VP, box);
			if (path_type == OPTIMAL_PATH) {
				render_optimal_path(cc, dc, VP, box);
			}
			else {
				WeatherUtils::draw_line_on_map(cc, dc, VP, box, conflict.latitude, conflict.longitude, refuge.latitude, refuge.longitude, green);
				render_conflicts(cc, dc, VP, box);
			}
		};

	private:
		void highlight_considered_grid(std::vector<std::vector<int>> &grid, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
			auto darkRedTransparent = wxColour(200, 0, 0, 150);

			for (int i = 0; i < grid.size(); i++) {
				for (int j = 0; j < grid[0].size(); j++) {
					double lat = get_coordinate_from_index(i, lat_min);
					double lon = get_coordinate_from_index(j, lon_min);
					if (grid[i][j] == 0 || grid[i][j] == 1) {
						WeatherUtils::print_zone(cc, dc, VP, box, lat, lon, darkRedTransparent);
					}
				}
			}
		}
		void render_optimal_path(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
			auto orange = wxColour(255, 165, 0, 255);
			for (int i = 0; i < optimal_path.size(); i++) {
				print_zone(cc, dc, VP, box,
					lat_min + ((double)optimal_path[i].second.first) / 10,
					lon_min + ((double)optimal_path[i].second.second) / 10,
					orange);
			}
		}

		void render_conflicts(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
			for each (auto item in conflicts)
			{
				// consider type for color
				print_zone(cc, dc, VP, box, item.latitude, item.longitude);
			}
		}

		bool IsSame(SimpleRoutePoint first, SimpleRoutePoint* second)
		{
			bool IsSame = false;

			if (fabs(first.latitude - second->latitude) < 1.e-6
				&& fabs(first.longitude - second->longitude) < 1.e-6) IsSame = true;
			return IsSame;
		}
	};
	bool CheckGetCachedRouteRefugeData(vector<RefugeRouteData>& items, SimpleRoutePoint* conflict, SimpleRoutePoint* refuge, ShipProperties& ship, int start_time_index, RefugeRouteData *dst);
}

#endif