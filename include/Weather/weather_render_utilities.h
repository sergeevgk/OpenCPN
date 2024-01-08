#ifndef __WEATHER_RENDER_UTILS__
#define __WEATHER_RENDER_UTILS__

class ChartCanvas;
class ViewPort;
class ocpnDCl;
class LLBBox;

namespace WeatherUtils {
	void print_zone(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double lat, double lon, wxColour colour = wxColour(135, 0, 135, 255));
	void draw_line_on_map(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double start_lat, double start_lon, double end_lat, double end_lon, wxColour color);
}

#endif