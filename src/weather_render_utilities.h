#ifndef __WEATHER_RENDER_UTILS__
#define __WEATHER_RENDER_UTILS__

class ChartCanvas;
class ViewPort;
class ocpnDCl;
class LLBBox;

namespace WeatherUtils {
	void print_zone(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double lat, double lon, wxColour colour = wxColour(135, 0, 135, 255));
}

#endif