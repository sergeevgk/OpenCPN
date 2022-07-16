#ifndef __WEATHER_UTILS_H__
#define __WEATHER_UTILS_H__

#include <wx/progdlg.h>

#include "vector2D.h"
#include "ocpndc.h"
#include "Route.h"

#include <vector>
#include <cmath>
#include <list>
#include <deque>
#include <queue>
#include <iostream>
#include <fstream>
#include <regex>
#include <utility>
#include <curl/curl.h>
#include <cm93.h>

class ChartCanvas;
class ViewPort;
class ocpnDCl;
class LLBBox;
class Route;

namespace WeatherUtils {

#define ZONE_WIDTH_DEFAULT 4 // стандартная ширина (в "клетках") зоны вокруг маршрута, в которой рассматриваем различные пути
					 // 1 клетка == 6 морских миль или 0.1 градуса

	// возможно надо различать северное и южное полушария чтобы поворачивать в нужную сторону
	// для единственного (на данный момент) использования метода это не играет роли
	wxPoint2DDouble rotate_vector_around_first_point(wxPoint2DDouble p1, wxPoint2DDouble p2, double angle);
	wxPoint2DDouble step_for_way(wxPoint2DDouble start, wxPoint2DDouble end, double step_length);
	bool is_in_range(wxPoint2DDouble point_a, wxPoint2DDouble point_b, double range);
	
	// @param v speed in knots
	// returns angle in radian
	double calculate_cone_angle(double v);
	void draw_considered_zone(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, std::list<std::pair<wxPoint2DDouble, wxPoint2DDouble>> zone_points);
	int get_coordinate_index(double coord, double coord_min);
	int get_time_index(int start_time_index, std::vector<std::string> all_choices, double start_time_three_hours_shift, double start_time_shift);
	std::pair<int, double> get_time_shift(double time);
	double get_coordinate_from_index(int index, double coord_min);
	double get_distance(wxPoint2DDouble a, wxPoint2DDouble b);

	class ConsideredZoneBuilder {
	public:
		ConsideredZoneBuilder();
		
		//coordinate_boundaries: [lat_min, lat_max, lon_min, lon_max]
		ConsideredZoneBuilder(int zone_width, double coordinate_boundaries[4]);
		void increase_zone_width(int times);
		std::vector<std::vector<int>> BuildConsideredZoneFromRoute(Route* route);
	private:
		int m_zone_width;
		double m_lat_min;
		double m_lat_max;
		double m_lon_min;
		double m_lon_max;

		void build_available_zone_for_section(double x0, double y0, double x1, double y1, std::vector<std::vector<int>> &grid);
	};
};

#endif