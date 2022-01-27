#ifndef __WEATHER_UTILS_H__
#define __WEATHER_UTILS_H__
#include "wx/geometry.h"

//class HyperlinkList;
//class ChartCanvas;
//class ViewPort;
//class ocpnDCl;
//class LLBBox;


namespace WeatherUtils {
	// возможно надо различать северное и южное полушария чтобы поворачивать в нужную сторону
	// для единственного (на данный момент) использования метода это не играет роли
	wxPoint2DDouble rotate_vector_around_first_point(wxPoint2DDouble p1, wxPoint2DDouble p2, double angle);
	wxPoint2DDouble step_for_way(wxPoint2DDouble start, wxPoint2DDouble end, double step_length);
	bool is_in_range(wxPoint2DDouble point_a, wxPoint2DDouble point_b, double range);
};

#endif