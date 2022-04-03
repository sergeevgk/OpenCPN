/*
Author:   Ilina Elena (ferr.98@mail.ru)
*/
#ifndef __WEATHER_H__
#define __WEATHER_H__

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
#include "cm93.h"
#include "db_utils.h"

class HyperlinkList;
class ChartCanvas;
class ViewPort;
class ocpnDCl;
class LLBBox;

//class DbUtils::DbContext;

class Weather
{
public:
	static std::string weather_dir_path;

	struct PointWeatherData {

		std::string creation_time;//replace
		std::string time;
		double latitude;
		double longitude;
		double wind_u;
		double wind_v;
		double wave_height;
		double wave_wind_length;
		double wave_wind_period;
		double wave_direction;
		double ripple_height;
		double ripple_direction;

		PointWeatherData() {
			creation_time = "-1";
		}

		PointWeatherData(const std::string &creationTime, const std::string &time, double latitude, double longitude,
			double windU, double windV, double waveHeight, double waveWindLength, double waveWindPeriod,
			double waveDirection, double rippleHeight, double rippleDirection) : creation_time(creationTime),
			time(time),
			latitude(latitude),
			longitude(longitude),
			wind_u(windU), wind_v(windV),
			wave_height(waveHeight),
			wave_wind_length(
				waveWindLength),
			wave_wind_period(
				waveWindPeriod),
			wave_direction(waveDirection),
			ripple_height(rippleHeight),
			ripple_direction(
				rippleDirection) {}


	};

	Weather();
	virtual ~Weather();

	void Draw(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
	std::vector<std::string> GetChoicesDateTime();

private:
	double do_work(const std::string& str);
	void draw_gradient(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
	void draw_refuge_places(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
	void print_zone(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double lat, double lon, wxColour colour = wxColour(135,0,135,255));
	void print_path_step(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double lat, double lon);
	void draw_check_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
	void draw_find_refuge_roots(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, RoutePoint currentPosition, double rescue_start_time);
	void draw_simple_refuge_root_with_conflicts(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route* route, std::vector<std::vector<int>> consideredZoneGrid, double rescue_start_time);
	void draw_simple_refuge_root_with_conflicts(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route* route, double rescue_start_time);
	void draw_calculate_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
	void draw_check_conflicts_on_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, std::vector<std::pair<double, std::pair<int, int>>> optimal_route);
	void get_all_weather_date_data(const std::string& path);
	double find_max_wave_height();
	double find_min_wave_height();
	double find_max_ripple_height();
	double find_min_ripple_height();
	double find_min_latitude();
	double find_max_latitude();
	double find_min_longitude();
	double find_max_longitude();


	static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
	static bool download_weather_from_esimo();
	void create_data_grid();
	void analyseRouteCheck(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route *route, double start_time_shift = 0);
	std::vector<std::pair<double, std::pair<int, int>>> find_fast_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route *route, std::vector<std::vector<int>> &considered_zone, double actual_start_time = 0);
	bool check_conflicts_in_weather_grid_cell(s57chart* chart, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, int lat_ind, int lon_ind);
	bool print_objects_values_to_file(ListOfObjRazRules* list, s57chart* chart);
	void check_land_collision(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double lat, double lon, s57chart* chart);
	bool is_deep_enough(ListOfObjRazRules *list, s57chart* chart, float draft);
	bool is_deep_enough_area(ListOfObjRazRules *list, s57chart* chart, float draft);
	void check_depth_in_cone(s57chart* chart, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, wxPoint2DDouble start, wxPoint2DDouble end);
	bool is_depth_in_cone_enough(s57chart* chart, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, wxPoint2DDouble start, wxPoint2DDouble end);
	bool is_land_area(ListOfObjRazRules *list, s57chart * chart);
	bool is_same_colour(wxColour first, wxColour second);
	ListOfObjRazRules * get_objects_at_lat_lon(double lat, double lon, double select_rad, s57chart* chart, ViewPort *VP, int mask = MASK_ALL);
	double calculate_speed_koef(ChartCanvas *cc, double h);
	bool is_in_weather_area(double lat1, double lon1);
	bool is_in_weather_grid(int lat_ind, int lon_ind);
	void highlight_considered_grid(std::vector<std::vector<int>> &grid, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
	int get_next_nearest_refuge_place_index(std::vector<WeatherUtils::RefugePlace> refuge_place_vector, RoutePoint position, int current_nearest_index);

private:
	 bool is_downloaded = false;
	 std::vector<std::pair<std::string, std::vector<PointWeatherData>>> date_data;
	 std::vector <std::pair<std::string, std::vector<std::vector<PointWeatherData>>>> grid_data;//25 блоков, каждый блок - пара из времени и двумерного массива поинтов
	 DbUtils::DbContext* db_context;
	 std::vector<WeatherUtils::RefugePlace> refuge_place_vector;
	 // optimal route which consists of pairs <time, pair <lat_index, lon_index> >
	 // lat/lon indices are from weather grid
	 std::vector<std::pair<double, std::pair<int, int>>> last_optimal_path;
	 bool draw_downloaded = false;

	 double lat_min;
	 double lat_max;
	 double lon_min;
	 double lon_max;
};
#endif