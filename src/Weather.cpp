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
extern float        g_ChartScaleFactorExp;
extern RouteList        *pRouteList;
extern TrackList        *pTrackList;

Weather::Weather()
{

	//Weather::download_weather_from_esimo();


	////data = get_all_weather_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\clean_data.csv");
	////data = get_all_weather_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\lena_test_data.csv");

	////get_all_weather_date_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\data_diff_days.csv");
	//get_all_weather_date_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\lena_test_data.csv");
	//
	////data = date_data[0].second;
	//if (data.size() > 0) {
	//	is_downloaded = true;
	//}
	//


	//dijstra test


	//data = get_all_weather_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\clean_data.csv");
	//data = get_all_weather_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\lena_test_data.csv");

	//get_all_weather_date_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\data_diff_days.csv");
	//get_all_weather_date_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\lena_test_data.csv");
	get_all_weather_date_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\clean_data.csv");
	create_data_grid();

	//data = date_data[0].second;
	if (date_data[0].second.size() > 0) {
		is_downloaded = true;
	}

}

Weather::~Weather(void)
{

}

void Weather::Draw(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box)
{
	draw_gradient(cc, dc, VP, box);
	if (cc->GetCheckRouteEnabled()) {
		draw_check_route(cc, dc, VP, box);
	}
	if (cc->GetCalculateRouteEnabled()) {
		draw_calculate_route(cc, dc, VP, box);
	}
}

void Weather::draw_check_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {

	if (cc->GetShipSpeed() <= 0) { return; }
	if (cc->GetStartTimeThreeHours() >= 3 * 60 * 60) return;
	if ((cc->GetStartTime() == "no data") || cc->GetStartTime() == "") return;
		 
	for (wxRouteListNode *node = pRouteList->GetFirst();
		node; node = node->GetNext()) {
		Route *pRouteDraw = node->GetData();

		if (!pRouteDraw)
			continue;

		/* defer rendering active routes until later */
		if (pRouteDraw->IsActive() || pRouteDraw->IsSelected())
			continue;

		/* defer rendering routes being edited until later */
		if (pRouteDraw->m_bIsBeingEdited)
			continue;

		if (pRouteDraw->pRoutePointList->empty()) continue;

		LLBBox weather_grid{};
		weather_grid.Set(lat_min, lon_min, lat_max, lon_max);

		if (weather_grid.IntersectOut(pRouteDraw->GetBBox())) continue;

		if (VP.GetBBox().IntersectOut(pRouteDraw->GetBBox()) || (!pRouteDraw->IsVisible())) continue;

		analyseRouteCheck(cc, dc, VP, box,pRouteDraw);
	}

	//if (is_downloaded) {
	//	std::string str = "               CHECK ROUTE " + std::to_string(cc->GetStartTimeThreeHours()) + " " + std::to_string(cc->GetShipDangerHeight()) + " " + std::to_string(cc->GetShipN()) + " " + std::to_string(cc->GetShipD()) + " " + std::to_string(cc->GetShipL()) + " " + std::to_string(cc->GetShipDelta());
	////	wxString msg = "               CHECK ROUTE";
	//	wxString msg(str);
	//	wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	//	dc.SetFont(*g_pFontSmall);
	//	wxColour cl = wxColour(61, 61, 204, 255);
	//	dc.SetTextForeground(cl);
	//	dc.DrawText(msg, 10, 10);
	//}
}

void Weather::analyseRouteCheck(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route *route) {

	wxRoutePointListNode *node = route->pRoutePointList->GetFirst();
	RoutePoint *prp2 = node->GetData();

	double sum_time = 0;//в часах
	double v = cc->GetShipSpeed();//////////////IMPLEMENT

	std::string start_time = cc->GetStartTime();
	
	double start_time_three_hours = cc->GetStartTimeThreeHours();
	double now_time_three_hours = start_time_three_hours / 60 / 60;//в часах
	std::string now_time = start_time;

	std::vector<std::string> all_choices = GetChoicesDateTime();
	std::sort(all_choices.begin(), all_choices.end());
	std::string errors = "\n\n\n";

	for (node = node->GetNext(); node; node = node->GetNext()) {
		RoutePoint *prp1 = prp2;
		prp2 = node->GetData();

		double lat1, lon1, lat2, lon2;
		lat1 = prp1->m_lat;
		lon1 = prp1->m_lon;
		lat2 = prp2->m_lat;
		lon2 = prp2->m_lon;


		double lat_start, lon_start;

		lat_start = std::round(lat1 * 10) / 10;
		lon_start = std::round(lon1 * 10) / 10;

		double lat_finish, lon_finish;

		lat_finish = std::round(lat2 * 10) / 10;
		lon_finish = std::round(lon2 * 10) / 10;
		 

		double norm_lat = 0.0, norm_lon = 0.0;//в какую сторону будет движение по сетке, нормаль то есть

		if (lat1 > lat2) {
			norm_lat = -0.1;
		}
		else if (lat1 < lat2){
			norm_lat = 0.1;
		}

		if (lon1 > lon2) {
			norm_lon = -0.1;
		}
		else if (lon1 < lon2){
			norm_lon = 0.1;
		}


		//то есть в чем идея: делим путь на три этапа: 1) из начальной точки до первой боковой грани 2) далее по граням движение 3) от последней грани до финишной точки

		//этап 1

		double k = 0;
		if (norm_lon != 0) {
			k = (lat2 - lat1) / (lon2 - lon1);
		}

		double b = 0;
		b = lat1 - k * lon1;

		double cos = (abs((lon1 - lon2))) / (sqrt((lon1 - lon2) * (lon1 - lon2) + (lat1 - lat2) * (lat1 - lat2)));
		double sin = (abs((lat1 - lat2))) / (sqrt((lon1 - lon2) * (lon1 - lon2) + (lat1 - lat2) * (lat1 - lat2)));

		double prev_lat = lat1, prev_lon = lon1;
		double prev_grid_lat = lat_start;
		double prev_grid_lon = lon_start;

		

		while (((std::round(prev_grid_lat * 10)) != std::round(lat_finish * 10)) || (std::round(prev_grid_lon * 10) != std::round(lon_finish * 10))) {

			double sum_waves = -1;
			//check for waves and others
			if (now_time != "no data" && now_time != "") {
				if ((prev_grid_lat >= lat_min) && (prev_grid_lat <= lat_max) && (prev_grid_lon >= lon_min) && (prev_grid_lon <= lon_max)) {
					int ind_lat, ind_lon;
					ind_lat = (prev_grid_lat - lat_min) * 10;
					ind_lon = (prev_grid_lon - lon_min) * 10;

					int ind_now_time = -1;
					for (int i = 0; i < grid_data.size(); i++) {
						if (grid_data[i].first == now_time) {
							ind_now_time = i;
							break;
						}
					}
					if (ind_now_time == -1) {
						errors += "ERRRRRROR";
					}
					if (ind_lat >= grid_data[ind_now_time].second.size()) {
						errors += "ERRRRRROR";
					}
					if (ind_lon >= grid_data[ind_now_time].second[ind_lat].size()) {
						errors += "ERRRRRROR";
					}
					PointWeatherData now_square = grid_data[ind_now_time].second[ind_lat][ind_lon];
					if (now_square.creation_time != "-1") {
						sum_waves = now_square.wave_height + now_square.ripple_height;

						if (sum_waves > ((double)cc->GetShipDangerHeight())/100) {
							std::string temp = "huge wave:" + std::to_string(sum_waves) + " in " + std::to_string(prev_lat) + " " + std::to_string(prev_lon);
							errors += temp + "\n";

							print_error_zone(cc, dc, VP, box, prev_lat, prev_lon);
						}
					}
				}
			}
			//
			double to_next_lon;
			double to_next_lat;
			bool is_lat_step = true;
			if (norm_lat == 0) is_lat_step = false;
			double lat_time = -1;
			double lon_time = -1;
			
			if (norm_lat != 0) {
				if (norm_lat < 0) {
					to_next_lat = prev_lat - (prev_grid_lat - 0.05);
					//to_next_lat = prev_grid_lat + 0.05 - prev_lat;
				}
				else {
					to_next_lat = prev_grid_lat + 0.05 - prev_lat;
					//to_next_lat = prev_lat - (prev_grid_lat - 0.05);
				}
				lat_time = (to_next_lat / sin) / v;
			}

			if (norm_lon != 0) {
				if (norm_lon < 0) {
					to_next_lon = prev_lon - (prev_grid_lon - 0.05);
					//to_next_lon = prev_grid_lon + 0.05 - prev_lon;
				}
				else {
					to_next_lon = prev_grid_lon + 0.05 - prev_lon;
					//to_next_lon = prev_lon - (prev_grid_lon - 0.05);
				}
				lon_time = (to_next_lon / cos) / v;
			}

			if (lat_time == -1) {
				is_lat_step = false;
			} else if (lon_time == -1) {
				is_lat_step = true;
			}
			else {
				is_lat_step = (lat_time < lon_time);
			}

			//можно уменьшить погрешность, если точку пересечения находить уже по уравнению начальному, но мне пофиг

			double delta_lat = 0;
			double delta_lon = 0;
			if (is_lat_step) {
				delta_lat = to_next_lat;
				if (norm_lon != 0) {
					delta_lon = to_next_lat * cos / sin;
				}
			}
			else {
				delta_lon = to_next_lon;
				if (norm_lat != 0) {
					delta_lat = to_next_lon * sin / cos;
				}
			}
			if (norm_lat > 0) {
				prev_lat += delta_lat;
				if (is_lat_step) {
					prev_grid_lat += 0.1;
					//if (prev_grid_lat > lat_finish - 0.1) break;
				}
			}
			else {
				prev_lat -= delta_lat;
				if (is_lat_step) {
					prev_grid_lat -= 0.1;
					//if (prev_grid_lat < lat_finish + 0.1) break;
				}
			}

			if (norm_lon > 0) {
				prev_lon += delta_lon;
				if (!is_lat_step) {
					prev_grid_lon += 0.1;
					//if (prev_grid_lon > lon_finish + 0.1) break;
				}
			}
			else {
				prev_lon -= delta_lon;
				if (!is_lat_step) {
					prev_grid_lon -= 0.1;
					//if (0.1 + prev_grid_lon < lon_finish) break;
				}
			}
			sum_time += std::min(lat_time, lon_time);
			//написать поддержку нужного времени - добавлять время и перекидывать на другой индекс
			now_time_three_hours += std::min(lat_time, lon_time);
			if (now_time_three_hours >= 3) {
				int spin = now_time_three_hours / 3;
				if (now_time == "no data") {
					now_time_three_hours -= spin * 3;
					continue;
				}

				int ind_now_time = -1;
				for (int i = 0; i < all_choices.size(); i++) {
					if (all_choices[i] == now_time) {
						ind_now_time = i;
						break;
					}
				}

				if (ind_now_time + spin >= all_choices.size()) {
					now_time = "no data";
				}
				else {
					now_time = all_choices[ind_now_time + spin];
				}
				now_time_three_hours -= spin * 3;
			}
		}

		double finishing_distance = sqrt((lon2 - prev_lon)*(lon2 - prev_lon) + (lat2 - prev_lat)*(lat2 - prev_lat));
		sum_time += finishing_distance / v;

		
	}

	if (is_downloaded) {
		std::string str = "                                   " + route->GetName() + "                                                                         " + std::to_string(sum_time) + errors;
		//	wxString msg = "               CHECK ROUTE";
		wxString msg(str);
		wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		dc.SetFont(*g_pFontSmall);
		wxColour cl = wxColour(61, 61, 204, 255);
		dc.SetTextForeground(cl);
		dc.DrawText(msg, 10, 10);
	}
}

bool Weather::is_deep_enough(double lat, double lon) {
	return true;
}

void Weather::print_error_zone(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double lat, double lon) {
	wxPoint r;
	wxRect hilitebox;

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

	int red, green, blue;
	green = 0;
	red = 135;
	blue = 135;

	wxColour hi_colour(red, green, blue, 255);
	//wxColour hi_colour = pen->GetColour();


	//  Highlite any selected point
	AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, radius,
		hi_colour, transparency);
}

void Weather::draw_calculate_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
	if (is_downloaded) {
		wxString msg = "\n               CALCULATE ROUTE";
		wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		dc.SetFont(*g_pFontSmall);
		wxColour cl = wxColour(61, 61, 204, 255);
		dc.SetTextForeground(cl);
		dc.DrawText(msg, 10, 10);
	}
}

void Weather::draw_gradient(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {

	std::string dateTime = cc->GetDateTime();

	if (dateTime == "no data") return;

	int index = -1;
	for (int i = 0; i < grid_data.size(); i++) {
		if (grid_data[i].first == dateTime) {
			index = i;
			break;
		}
	}
	if (index == -1) return;

	std::vector<std::vector<PointWeatherData>> now_data = grid_data[index].second;


	for (int i = 0; i < now_data.size(); i++) {
		for (int j = 0; j < now_data[i].size(); j++) {
			wxPoint r;
			wxRect hilitebox;

			double lat, lon;
			if (now_data[i][j].creation_time == "-1") continue;
			lat = now_data[i][j].latitude;
			lon = now_data[i][j].longitude;

			cc->GetCanvasPointPix(lat, lon, &r);

			wxPen *pen;
			pen = g_pRouteMan->GetRoutePointPen();

			int sx2 = 2;
			int sy2 = 2;

			wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);           // the bitmap extents

			hilitebox = r1;
			hilitebox.x -= r.x;
			hilitebox.y -= r.y;

			
			//hilitebox.x *= g_ChartScaleFactorExp;
			//hilitebox.y *= g_ChartScaleFactorExp;
			hilitebox.width *= g_ChartScaleFactorExp;
			hilitebox.height *= g_ChartScaleFactorExp;

			hilitebox.width *= VP.view_scale_ppm;
			hilitebox.height *= VP.view_scale_ppm;

			float radius;
			hilitebox.Inflate(4);
			radius = 1.0f;

			unsigned char transparency = 100;

			int red, green, blue;
			double min_value_wave = find_min_wave_height();
			double max_value_wave = find_max_wave_height();
			double min_value_ripple = find_min_ripple_height();
			double max_value_ripple = find_max_ripple_height();
			green = 0;
			red = 0;
			blue = 0;

			int mode = cc->GetWeatherHeightMode();
			double now, min_value, max_value;
			if (mode == WAVE_HEIGHT) {
				now = now_data[i][j].wave_height;
				min_value = min_value_wave;
				max_value = max_value_wave;

			}
			else if (mode == RIPPLE_HEIGHT) {
				now = now_data[i][j].ripple_height;
				min_value = min_value_ripple;
				max_value = max_value_ripple;
			}
			else {
				now = now_data[i][j].wave_height + now_data[i][j].ripple_height;
				min_value = min_value_wave + min_value_ripple;
				max_value = max_value_wave + max_value_ripple;
			}
			double danger = cc->GetDangerHeight();
			double range = max_value - min_value;
			if (danger > max_value) {
				range = std::max(danger - min_value, max_value - danger);
			}

			if (now < danger) {
				green = (danger - now) * 255 / (range);
			}
			else {
				red = (range - (now - danger)) * 255 / (range);
				transparency = 180;
			}



			wxColour hi_colour(red, green, blue, 255);
			//wxColour hi_colour = pen->GetColour();


			//  Highlite any selected point
			AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, radius,
				hi_colour, transparency);

			//// написать текст в верхнем левом углу (отлияный способ быстро понять, что что-то работает)
			//if (is_downloaded) {
			//	wxString msg = "I was here";
			//	wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
			//	dc.SetFont(*g_pFontSmall);
			//	wxColour cl = wxColour(61, 61, 204, 255);
			//	dc.SetTextForeground(cl);
			//	dc.DrawText(msg, 10, 10);
			//}
		}
	}
}

//void Weather::draw_gradient(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
//	for (int i = 0; i < data.size(); i++) {
//		wxPoint r;
//		wxRect hilitebox;
//
//		double lat, lon;
//		lat = data[i].latitude;
//		lon = data[i].longitude;
//
//		cc->GetCanvasPointPix(lat, lon, &r);
//
//		wxPen *pen;
//		pen = g_pRouteMan->GetRoutePointPen();
//
//		int sx2 = 2;
//		int sy2 = 2;
//
//		wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);           // the bitmap extents
//
//		hilitebox = r1;
//		hilitebox.x -= r.x;
//		hilitebox.y -= r.y;
//		float radius;
//		hilitebox.Inflate(4);
//		radius = 1.0f;
//
//		int red, green, blue;
//		double min_value = find_min_wave_height();
//		double max_value = find_max_wave_height();
//		red = 0;
//		blue = 0;
//		double now = data[i].wave_height;
//		green = (now - min_value) * 255 / (max_value - min_value);
//		wxColour hi_colour( red, green, blue, 255 );
//		//wxColour hi_colour = pen->GetColour();
//		unsigned char transparency = 100;
//
//		//  Highlite any selected point
//		AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, radius,
//			hi_colour, transparency);
//
//		//// написать текст в верхнем левом углу (отлияный способ быстро понять, что что-то работает)
//		//if (is_downloaded) {
//		//	wxString msg = "I was here";
//		//	wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
//		//	dc.SetFont(*g_pFontSmall);
//		//	wxColour cl = wxColour(61, 61, 204, 255);
//		//	dc.SetTextForeground(cl);
//		//	dc.DrawText(msg, 10, 10);
//		//}
//	}
//}

double Weather::do_work(const std::string& str) {
	if (str.empty()) {
		return 0.0;
	}
	return stod(str);
}

double Weather::find_max_wave_height() {
	if (date_data.size() == 0) {
		return -1;
	}

	if (date_data[0].second.size() == 0) return -1;
	double max = date_data[0].second[0].wave_height;

	for (int ind = 0; ind < date_data.size(); ind++) {
		for (int i = 0; i < date_data[ind].second.size(); i++) {
			double temp = date_data[ind].second[i].wave_height;
			if (temp > max) {
				max = temp;
			}
		}
	}
	return max;
}

double Weather::find_min_wave_height() {
	if (date_data.size() == 0) {
		return -1;
	}

	if (date_data[0].second.size() == 0) return -1;
	double min = date_data[0].second[0].wave_height;

	for (int ind = 0; ind < date_data.size(); ind++) {
		for (int i = 0; i < date_data[ind].second.size(); i++) {
			double temp = date_data[ind].second[i].wave_height;
			if (temp < min) {
				min = temp;
			}
		}
	}
	return min;
}

double Weather::find_max_ripple_height() {
	if (date_data.size() == 0) {
		return -1;
	}

	if (date_data[0].second.size() == 0) return -1;
	double max = date_data[0].second[0].ripple_height;

	for (int ind = 0; ind < date_data.size(); ind++) {
		for (int i = 0; i < date_data[ind].second.size(); i++) {
			double temp = date_data[ind].second[i].ripple_height;
			if (temp > max) {
				max = temp;
			}
		}
	}
	return max;
}

double Weather::find_min_ripple_height() {
	if (date_data.size() == 0) {
		return -1;
	}

	if (date_data[0].second.size() == 0) return -1;
	double min = date_data[0].second[0].ripple_height;

	for (int ind = 0; ind < date_data.size(); ind++) {
		for (int i = 0; i < date_data[ind].second.size(); i++) {
			double temp = date_data[ind].second[i].ripple_height;
			if (temp < min) {
				min = temp;
			}
		}
	}
	return min;
}

double Weather::find_max_latitude() {
	if (date_data.size() == 0) {
		return -1;
	}

	if (date_data[0].second.size() == 0) return -1;
	double max = date_data[0].second[0].latitude;

	for (int ind = 0; ind < date_data.size(); ind++) {
		for (int i = 0; i < date_data[ind].second.size(); i++) {
			double temp = date_data[ind].second[i].latitude;
			if (temp > max) {
				max = temp;
			}
		}
	}
	return max;
}

double Weather::find_min_latitude() {
	if (date_data.size() == 0) {
		return -1;
	}

	if (date_data[0].second.size() == 0) return -1;
	double min = date_data[0].second[0].latitude;

	for (int ind = 0; ind < date_data.size(); ind++) {
		for (int i = 0; i < date_data[ind].second.size(); i++) {
			double temp = date_data[ind].second[i].latitude;
			if (temp < min) {
				min = temp;
			}
		}
	}
	return min;
}

double Weather::find_max_longitude() {
	if (date_data.size() == 0) {
		return -1;
	}

	if (date_data[0].second.size() == 0) return -1;
	double max = date_data[0].second[0].longitude;

	for (int ind = 0; ind < date_data.size(); ind++) {
		for (int i = 0; i < date_data[ind].second.size(); i++) {
			double temp = date_data[ind].second[i].longitude;
			if (temp > max) {
				max = temp;
			}
		}
	}
	return max;
}

double Weather::find_min_longitude() {
	if (date_data.size() == 0) {
		return -1;
	}

	if (date_data[0].second.size() == 0) return -1;
	double min = date_data[0].second[0].longitude;

	for (int ind = 0; ind < date_data.size(); ind++) {
		for (int i = 0; i < date_data[ind].second.size(); i++) {
			double temp = date_data[ind].second[i].longitude;
			if (temp < min) {
				min = temp;
			}
		}
	}
	return min;
}

void Weather::get_all_weather_date_data(const std::string& path) {
	std::string line;
	// std::vector<PointWeatherData> all_data;

	std::ifstream in(path);
	if (in.is_open())
	{
		getline(in, line);
		const std::regex r(R"(\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\")");


		while (getline(in, line))
		{

			std::smatch m;
			std::regex_search(line, m, r);
			std::vector<double> temp;

			PointWeatherData data;
			data.creation_time = m[1].str();
			data.time = m[2].str();


			data.latitude = do_work(m[3].str());
			data.longitude = do_work(m[4].str());
			data.wind_u = do_work(m[5].str());
			data.wind_v = do_work(m[6].str());
			data.wave_height = do_work(m[7].str());
			data.wave_wind_length = do_work(m[8].str());
			data.wave_wind_period = do_work(m[9].str());
			data.wave_direction = do_work(m[10].str());
			data.ripple_height = do_work(m[11].str());
			data.ripple_direction = do_work(m[12].str());


			int index = -1;
			for (int i = 0; i < date_data.size(); i++) {
				if (date_data[i].first == data.time) {
					index = i;
					break;
				}
			}
			if (index != -1) {
				date_data[index].second.push_back(data);
			}
			else {
				std::vector<PointWeatherData> temp = { data };
				date_data.push_back({ data.time, temp });
			}
		}
	}
	in.close();
}

std::vector<std::string> Weather::GetChoicesDateTime() {
	std::vector<std::string> choices;
	for (int i = 0; i < date_data.size(); i++) {
		choices.push_back(date_data[i].first);
	}
	return choices;
}

void Weather::create_data_grid() {
	lat_min = find_min_latitude();
	lat_max = find_max_latitude();
	lon_min = find_min_longitude();
	lon_max = find_max_longitude();

	int lat_size = (lat_max - lat_min) * 10 + 1;
	int lon_size = (lon_max - lon_min) * 10 + 1;

	
	for (int i = 0; i < date_data.size(); i++) {
		std::vector<std::vector<PointWeatherData>> t;//lat --- lon
		for (int j = 0; j < lat_size; j++) {
			std::vector<PointWeatherData> temp;
			t.push_back(temp);
			t[t.size() - 1].resize(lon_size);
		}

		for (int ind = 0; ind < date_data[i].second.size(); ind++) {
			int lat_ind = (date_data[i].second[ind].latitude - lat_min) * 10;
			int lon_ind = (date_data[i].second[ind].longitude - lon_min) * 10;
			//if (lat_ind >= lat_size) continue;
			/*if (t[lat_ind].size() == 0) {
				t[lat_ind].resize(lon_size);
			}*/
			t[lat_ind][lon_ind] = date_data[i].second[ind];
		}
		grid_data.push_back({ date_data[i].first, t });
	}
}


size_t Weather::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

bool Weather::download_weather_from_esimo() {
	CURL *curl;
	FILE *fp;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		fp = fopen("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\lena_test_data.csv", "wb");
		//fp = fopen("C:\\Users\\Admin\\Desktop\\TEST\\lena_test_data.csv", "wb");
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_URL, "http://esimo.ru/dataview/getresourceexport");
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "format=csv&resourceid=RU_Hydrometcentre_69&filter=");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(fp);
		return true;
	}
	return false;
}