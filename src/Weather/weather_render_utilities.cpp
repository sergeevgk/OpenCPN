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
