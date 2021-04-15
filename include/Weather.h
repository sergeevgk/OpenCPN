/*
Author:   Ilina Elena (ferr.98@mail.ru)
*/
#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <wx/progdlg.h>

#include "vector2D.h"
#include "ocpndc.h"

#include <vector>
#include <list>
#include <deque>

class HyperlinkList;
class ChartCanvas;
class ViewPort;
class ocpnDCl;
class LLBBox;

class Weather
{
public:
	Weather();
	virtual ~Weather();

	static void Draw(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box);
};
#endif