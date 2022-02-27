#include "wx/wxprec.h"

#include "weather_utils.h"
#include "routeman.h"
#include "navutil.h"
#include "chcanv.h"
#include "Route.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
extern ocpnGLOptions g_GLOptions;
#endif

extern Routeman *g_pRouteMan;
extern float        g_ChartScaleFactorExp;
extern RouteList        *pRouteList;
extern TrackList        *pTrackList;

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

// @param v speed in knots
// returns angle in radian
double WeatherUtils::calculate_cone_angle(double v) {
	const double k = 0.157 / 5; //соотношение скорости и отклонения
	return v * k;
}

void WeatherUtils::draw_line_on_map(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double start_lat, double start_lon, double end_lat, double end_lon, wxColour color) {
	int transparency = 255;
	wxDC *pdc = dc.GetDC();
	wxPoint p_start;
	wxPoint p_end;
	cc->GetCanvasPointPix(start_lat, start_lon, &p_start);
	cc->GetCanvasPointPix(end_lat, end_lon, &p_end);
	int x = p_start.x;
	int y = p_start.y;
	double size_x = std::abs(p_end.x - p_start.x);
	double size_y = std::abs(p_end.y - p_start.y);
	if (pdc) {

		//    Get wxImage of area of interest
		wxBitmap obm(size_x, size_y);
		wxMemoryDC mdc1;
		mdc1.SelectObject(obm);
		mdc1.Blit(0, 0, size_x, size_y, pdc, x, y);
		mdc1.SelectObject(wxNullBitmap);
		wxImage oim = obm.ConvertToImage();

		//    Create destination image
		wxBitmap olbm(size_x, size_y);
		wxMemoryDC oldc(olbm);
		if (!oldc.IsOk())
			return;

		oldc.SetBackground(*wxBLACK_BRUSH);
		oldc.SetBrush(*wxWHITE_BRUSH);
		oldc.Clear();

		oldc.DrawLine(0, 0, size_x, size_y);

		wxImage dest = olbm.ConvertToImage();
		unsigned char *dest_data = (unsigned char *)malloc(
			size_x * size_y * 3 * sizeof(unsigned char));
		unsigned char *bg = oim.GetData();
		unsigned char *box = dest.GetData();
		unsigned char *d = dest_data;

		//  Sometimes, on Windows, the destination image is corrupt...
		if (NULL == box)
		{
			free(d);
			return;
		}
		float alpha = 1.0 - (float)transparency / 255.0;
		int sb = size_x * size_y;
		for (int i = 0; i < sb; i++) {
			float a = alpha;
			if (*box == 0) a = 1.0;
			int r = ((*bg++) * a) + (1.0 - a) * color.Red();
			*d++ = r; box++;
			int g = ((*bg++) * a) + (1.0 - a) * color.Green();
			*d++ = g; box++;
			int b = ((*bg++) * a) + (1.0 - a) * color.Blue();
			*d++ = b; box++;
		}

		dest.SetData(dest_data);

		//    Convert destination to bitmap and draw it
		wxBitmap dbm(dest);
		dc.DrawBitmap(dbm, x, y, false);

		// on MSW, the dc Bounding box is not updated on DrawBitmap() method.
		// Do it explicitely here for all platforms.
		dc.CalcBoundingBox(x, y);
		dc.CalcBoundingBox(x + size_x, y + size_y);
	}
	else {
#ifdef ocpnUSE_GL
		/* opengl version */
		glEnable(GL_BLEND);
		wxColour c(color.Red(), color.Green(), color.Blue(), transparency);
		dc.SetPen(wxPen(c, 2));
		dc.DrawLine(p_start.x, p_start.y, p_end.x, p_end.y);
		glDisable(GL_BLEND);
#endif
	}
}

std::pair<wxPoint2DDouble, wxPoint2DDouble> get_zone_points(RoutePoint* p1, RoutePoint* p2, double zone_width)
{
	double dx = p1->m_lon - p2->m_lon; // Ay-By
	double dy = p2->m_lat - p1->m_lat; // Bx-Ax
	wxPoint2DDouble resultPoint1 = wxPoint2DDouble(p1->m_lat, p1->m_lon) + wxPoint2DDouble(dx, dy);
	wxPoint2DDouble resultPoint2 = wxPoint2DDouble(p1->m_lat, p1->m_lon) - wxPoint2DDouble(dx, dy);
	return std::pair<wxPoint2DDouble, wxPoint2DDouble>(resultPoint1, resultPoint2);
}

std::list<std::pair<wxPoint2DDouble, wxPoint2DDouble>> WeatherUtils::create_considered_zone_from_route(Route* route)
{
	std::list<std::pair<wxPoint2DDouble, wxPoint2DDouble>> resultList = std::list<std::pair<wxPoint2DDouble, wxPoint2DDouble>>();
	wxRoutePointListNode* prevNode = route->pRoutePointList->GetFirst();
	double zone_width = ZONE_WIDTH * 0.1;

	for (wxRoutePointListNode* node = prevNode->GetNext();
		node;
		node = node->GetNext()) 
	{
		RoutePoint* pPrevRoutePoint = prevNode->GetData();
		RoutePoint *pRoutePoint = node->GetData();
		
		auto zone_points = get_zone_points(pPrevRoutePoint, pRoutePoint, zone_width);
		
		resultList.push_back(zone_points);

		prevNode = node;
	}
}

void WeatherUtils::draw_considered_zone(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, std::list<std::pair<wxPoint2DDouble, wxPoint2DDouble>> zone_points) {
	std::pair<wxPoint2DDouble, wxPoint2DDouble> pointPairPrev = zone_points.front();
	wxColour zone_color = wxColour(0, 255, 0, 255);
	zone_points.pop_front();
	for (std::pair<wxPoint2DDouble, wxPoint2DDouble> pointPair : zone_points) {
		draw_line_on_map(cc, dc, VP, box, pointPairPrev.first.m_x, pointPairPrev.first.m_y, pointPair.first.m_x, pointPair.first.m_y, zone_color);
		draw_line_on_map(cc, dc, VP, box, pointPairPrev.second.m_x, pointPairPrev.second.m_y, pointPair.second.m_x, pointPair.second.m_y, zone_color);
		pointPairPrev = pointPair;
	}
}

void print_to_file(RoutePoint *p, std::string filename) {
	std::ofstream myfile;
	myfile.open(filename, std::ofstream::app);
	myfile << p->GetLatitude();
	myfile << " - ";
	myfile << p->GetLongitude();
	myfile << "\n";
	myfile.close();
}

std::vector<std::vector<int>> WeatherUtils::create_considered_grid_from_route(Route* route, double lat_min, double lat_max, double lon_min, double lon_max)
{
	wxRoutePointListNode *node_start = route->pRoutePointList->GetFirst();
	RoutePoint *start = node_start->GetData();
	wxRoutePointListNode *node_finish = route->pRoutePointList->GetLast();
	RoutePoint *finish = node_finish->GetData();

	double start_grid_lat = std::round(start->m_lat * 10) / 10;
	double start_grid_lon = std::round(start->m_lon * 10) / 10;
	double finish_grid_lat = std::round(finish->m_lat * 10) / 10;
	double finish_grid_lon = std::round(finish->m_lon * 10) / 10;

	std::vector<std::vector<int>> resultGrid = std::vector<std::vector<int>>();
	int lat_size = (lat_max - lat_min) * 10 + 1;
	int lon_size = (lon_max - lon_min) * 10 + 1;
	for (int j = 0; j < lat_size; j++) {
		std::vector<int> temp;
		resultGrid.push_back(temp);
		resultGrid[resultGrid.size() - 1].resize(lon_size, -1);
	}

	wxRoutePointListNode* prevNode = route->pRoutePointList->GetFirst();

	for (wxRoutePointListNode* node = prevNode->GetNext();
		node;
		node = node->GetNext())
	{
		RoutePoint* pPrevRoutePoint = prevNode->GetData();
		RoutePoint *pRoutePoint = node->GetData();
	/*	print_to_file(pRoutePoint, "C:\\Users\\gosha\\Documents\\GitHub\\openCPN_temp\\tmepFile.txt");
		print_to_file(pPrevRoutePoint, "C:\\Users\\gosha\\Documents\\GitHub\\openCPN_temp\\tmepFile.txt");*/

		double lat1, lon1, lat2, lon2;
		lat1 = pPrevRoutePoint->m_lat;
		lon1 = pPrevRoutePoint->m_lon;
		lat2 = pRoutePoint->m_lat;
		lon2 = pRoutePoint->m_lon;
		

		double lat_start, lon_start;

		lat_start = std::round(lat1 * 10) / 10;
		lon_start = std::round(lon1 * 10) / 10;

		double lat_finish, lon_finish;

		lat_finish = std::round(lat2 * 10) / 10;
		lon_finish = std::round(lon2 * 10) / 10;


		build_available_zone_for_section(lat_start, lon_start, lat_finish, lon_finish, lat_min, lon_min, resultGrid);

		prevNode = node;
	}
	return resultGrid;
}

void propagade_evenly_with_size(std::vector<std::vector<int>> &grid, int size, int x, int y)
{
	int dim_x = grid.size();
	int dim_y = grid[0].size();
	int i, j;
	for (i = size; i >= 0; i--) {
		for (j = 0; j <= size - i; j++) {
			if (x + i < dim_x && y + j < dim_y && grid[x + i][y + j] != 0)
				grid[x + i][y + j] = 1;
			if (x + i < dim_x && y - j >= 0 && grid[x + i][y - j] != 0)
				grid[x + i][y - j] = 1;
			if (x - i >= 0 && y + j < dim_y && grid[x - i][y + j] != 0)
				grid[x - i][y + j] = 1;
			if (x - i >= 0 && y - j >= 0 && grid[x - i][y - j] != 0)
				grid[x - i][y - j] = 1;
		}
	}
}

void WeatherUtils::build_available_zone_for_section(double lat_start, double lon_start, double lat_finish, double lon_finish, double lat_min, double lon_min, std::vector<std::vector<int>> &grid)
{
	int start_lat_idx = get_coordinate_index(lat_start, lat_min);
	int start_lon_idx = get_coordinate_index(lon_start, lon_min);
	int finish_lat_idx = get_coordinate_index(lat_finish, lat_min);
	int finish_lon_idx = get_coordinate_index(lon_finish, lon_min);

	double dx = fabs(lat_finish - lat_start);
	double dy = fabs(lon_finish - lon_start);

	/*int x = int(floor(lat_start));
	int y = int(floor(lon_start));*/
	int x = std::round(lat_start * 10);
	int y = std::round(lon_start * 10);

	double dt_dx = 1.0 / dx;
	double dt_dy = 1.0 / dy;

	double t = 0;

	int n = 1;
	int x_inc, y_inc;
	double t_next_vertical, t_next_horizontal;

	if (dx == 0)
	{
		x_inc = 0;
		t_next_horizontal = dt_dx; // infinity
	}
	else if (lat_finish > lat_start)
	{
		x_inc = 1;
		n += finish_lat_idx - start_lat_idx;
		t_next_horizontal = (floor(lat_start) + 1 - lat_start) * dt_dx;
	}
	else
	{
		x_inc = -1;
		n += start_lat_idx - finish_lat_idx;
		t_next_horizontal = (lat_start - floor(lat_start)) * dt_dx;
	}

	if (dy == 0)
	{
		y_inc = 0;
		t_next_vertical = dt_dy; // infinity
	}
	else if (lon_finish > lon_start)
	{
		y_inc = 1;
		n += finish_lon_idx - start_lon_idx;
		t_next_vertical = (floor(lon_start) + 1 - lon_start) * dt_dy;
	}
	else
	{
		y_inc = -1;
		n += start_lon_idx - finish_lon_idx;
		t_next_vertical = (lon_start - floor(lon_start)) * dt_dy;
	}

	for (; n > 0; --n)
	{
		grid[x][y] = 0;
		propagade_evenly_with_size(grid, ZONE_WIDTH, x, y);
		if (t_next_vertical < t_next_horizontal)
		{
			y += y_inc;
			t = t_next_vertical;
			t_next_vertical += dt_dy;
		}
		else
		{
			x += x_inc;
			t = t_next_horizontal;
			t_next_horizontal += dt_dx;
		}
	}
}


//int WeatherUtils::get_lat_index(double lat, double lat_min) {
//	lat = std::round(lat * 10) / 10;
//	return (lat - lat_min) * 10;
//}
//
//int WeatherUtils::get_lon_index(double lon, double lon_min) {
//	lon = std::round(lon * 10) / 10;
//	return (lon - lon_min) * 10;
//}

int WeatherUtils::get_coordinate_index(double coord, double coord_min) {
	coord = std::round(coord * 10) / 10;
	return (coord - coord_min) * 10;
}

double WeatherUtils::get_coordinate_from_index(int index, double coord_min) {
	double coord = (double)index / 10.0f + coord_min;
	return coord;
}
