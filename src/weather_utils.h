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
#include "cm93.h"

class ChartCanvas;
class ViewPort;
class ocpnDCl;
class LLBBox;
class Route;

namespace WeatherUtils {

#define ZONE_WIDTH 6 // ������ (� "�������") ���� ������ ��������, � ������� ������������� ��������� ����
					 // 1 ������ == 6 ������� ���� ��� 0.1 �������

	// �������� ���� ��������� �������� � ����� ��������� ����� ������������ � ������ �������
	// ��� ������������� (�� ������ ������) ������������� ������ ��� �� ������ ����
	wxPoint2DDouble rotate_vector_around_first_point(wxPoint2DDouble p1, wxPoint2DDouble p2, double angle);
	wxPoint2DDouble step_for_way(wxPoint2DDouble start, wxPoint2DDouble end, double step_length);
	bool is_in_range(wxPoint2DDouble point_a, wxPoint2DDouble point_b, double range);
	
	// @param v speed in knots
	// returns angle in radian
	double calculate_cone_angle(double v);

	void draw_line_on_map(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double start_lat, double start_lon, double end_lat, double end_lon, wxColour color);
	void draw_considered_zone(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, std::list<std::pair<wxPoint2DDouble, wxPoint2DDouble>> zone_points);
	std::list<std::pair<wxPoint2DDouble, wxPoint2DDouble>> create_considered_zone_from_route(Route* route);
	std::list<wxPoint2DDouble> create_considered_grid_from_route(Route* route);
};

#endif