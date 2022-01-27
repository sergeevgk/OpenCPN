#include "weather_utils.h"

wxPoint2DDouble WeatherUtils::rotate_vector_around_first_point(wxPoint2DDouble p1, wxPoint2DDouble p2, double angle)
{
	double x_rotated = ((p2.m_x - p1.m_x) * cos(angle)) - ((p2.m_y - p1.m_y) * sin(angle)) + p1.m_x;
	double y_rotated = ((p2.m_x - p1.m_x) * sin(angle)) + ((p2.m_y - p1.m_y) * cos(angle)) + p1.m_y;
	return wxPoint2DDouble(x_rotated, y_rotated);
}

wxPoint2DDouble WeatherUtils::step_for_way(wxPoint2DDouble start, wxPoint2DDouble end, double step_length)
{
	auto temp = (end - start);
	temp.Normalize();
	return wxPoint2DDouble(temp.m_x * step_length, temp.m_y * step_length);
}

bool WeatherUtils::is_in_range(wxPoint2DDouble point_a, wxPoint2DDouble point_b, double range)
{
	return point_a.GetDistance(point_b) <= range;
}
