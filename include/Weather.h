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
#include <iostream>
#include <fstream>
#include <regex>
#include <utility>

#include <curl/curl.h>

class HyperlinkList;
class ChartCanvas;
class ViewPort;
class ocpnDCl;
class LLBBox;

class Weather
{
public:

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
	void draw_check_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
	void draw_calculate_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
	std::vector<PointWeatherData> get_all_weather_data(const std::string& path);
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
	void draw_path(); 
	void create_data_grid();
	void analyseRouteCheck(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route *route);
	bool is_deep_enough(double lat, double lon);

private:
	 bool is_downloaded = false;
	 //std::vector<PointWeatherData> data;
	 std::vector<std::pair<std::string, std::vector<PointWeatherData>>> date_data;
	 std::vector <std::pair<std::string, std::vector<std::vector<PointWeatherData>>>> grid_data;//25 блоков, каждый блок - пара из времени и двумерного массива поинтов


	 bool draw_downloaded = false;

	 double lat_min;
	 double lat_max;
	 double lon_min;
	 double lon_max;
};
#endif