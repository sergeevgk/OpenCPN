
#include "wx/wxprec.h"

#include "Weather.h"

#include "Route.h"
#include "Track.h"
#include "routeman.h"
#include "ocpndc.h"
#include "georef.h"
#include "chartbase.h"
#include "navutil.h"
#include "Select.h"
#include "chcanv.h"

#include "pluginmanager.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
extern ocpnGLOptions g_GLOptions;
#endif

extern WayPointman *pWayPointMan;
extern Routeman *g_pRouteMan;
extern Select *pSelect;
extern MyConfig *pConfig;
extern double gLat, gLon;
extern double           g_PlanSpeed;
extern int              g_nTrackPrecision;
extern bool             g_bTrackDaily;
extern bool             g_bHighliteTracks;
extern double           g_TrackDeltaDistance;
extern float            g_GLMinSymbolLineWidth;
extern wxColour         g_colourTrackLineColour;
extern PlugInManager    *g_pi_manager;
extern wxColor GetDimColor(wxColor c);

#if defined( __UNIX__ ) && !defined(__WXOSX__)  // high resolution stopwatch for profiling
class OCPNStopWatch
{
public:
	OCPNStopWatch() { Reset(); }
	void Reset() { clock_gettime(CLOCK_REALTIME, &tp); }

	double GetTime() {
		timespec tp_end;
		clock_gettime(CLOCK_REALTIME, &tp_end);
		return (tp_end.tv_sec - tp.tv_sec) * 1.e3 + (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
	}

private:
	timespec tp;
};
#endif

Weather::Weather()
{

}

Weather::~Weather(void)
{

}

void Weather::Draw(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box)
{
	wxPoint r;
	wxRect hilitebox;

	cc->GetCanvasPointPix(38, 40, &r);

	wxPen *pen;
	pen = g_pRouteMan->GetRoutePointPen();

	int sx2 = 8;
	int sy2 = 8;

	wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);           // the bitmap extents

	hilitebox = r1;
	hilitebox.x -= r.x;
	hilitebox.y -= r.y;
	float radius;
	hilitebox.Inflate(4);
	radius = 4.0f;

	wxColour hi_colour = pen->GetColour();
	unsigned char transparency = 100;

	//  Highlite any selected point
	AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, radius,
		hi_colour, transparency);

	// написать текст в верхнем левом углу (отлияный способ быстро понять, что что-то работает)
	/*wxString msg = "I was here";
	wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	dc.SetFont(*g_pFontSmall);
	wxColour cl = wxColour(61, 61, 204, 255);
	dc.SetTextForeground(cl);
	dc.DrawText(msg,10,10);*/
}