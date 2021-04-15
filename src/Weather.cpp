#include "wx/wxprec.h"

#include "Weather.h"

#include "routeman.h"
#include "navutil.h"
#include "chcanv.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
extern ocpnGLOptions g_GLOptions;
#endif

extern Routeman *g_pRouteMan;

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

	int sx2 = 2;
	int sy2 = 2;

	wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);           // the bitmap extents

	hilitebox = r1;
	hilitebox.x -= r.x;
	hilitebox.y -= r.y;
	float radius;
	hilitebox.Inflate(4);
	radius = 1.0f;

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