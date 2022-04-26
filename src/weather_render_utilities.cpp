#include "wx/wxprec.h"
#include "routeman.h"
#include "navutil.h"
#include "chcanv.h"
#include "Route.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
extern ocpnGLOptions g_GLOptions;
#endif

extern Routeman *g_pRouteMan;

void WeatherUtils::print_zone(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double lat, double lon, wxColour fill_colour) {
	wxPoint r;
	wxRect hilitebox;
	wxColour default_colour = wxColour(0, 0, 0, 0);

	cc->GetCanvasPointPix(lat, lon, &r);

	wxPen *pen;
	pen = g_pRouteMan->GetRoutePointPen();

	int sx2 = 2;
	int sy2 = 2;

	wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);           // the bitmap extents

	hilitebox = r1;
	hilitebox.x -= r.x;
	hilitebox.y -= r.y;

	float radius;
	hilitebox.Inflate(4);
	radius = 1.0f;

	unsigned char transparency = 200;
	AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, radius,
		fill_colour, transparency);
}