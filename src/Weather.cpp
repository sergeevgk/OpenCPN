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

	//get_all_weather_date_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\clean_data.csv");
	get_all_weather_date_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\3three_data.csv");
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
	double v_nominal = cc->GetShipSpeed();//////////////IMPLEMENT
	double v = -1;

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
			v = -1;
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
					PointWeatherData now_square = grid_data[ind_now_time].second[ind_lat][ind_lon];
					
					if (now_square.creation_time != "-1") {
						sum_waves = now_square.wave_height + now_square.ripple_height;
						v = v_nominal * calculate_speed_koef(cc, sum_waves);
						if (sum_waves > ((double)cc->GetShipDangerHeight())/100 || !is_deep_enough(prev_lat, prev_lon)) {
							std::string temp = "huge wave:" + std::to_string(sum_waves) + " in " + std::to_string(prev_lat) + " " + std::to_string(prev_lon);
							errors += temp + "\n";

							print_error_zone(cc, dc, VP, box, prev_lat, prev_lon);
						}
					}
				}
			}
			//
			if (v == -1) {
				v = v_nominal;
			}
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
		std::string str = "                                   " + route->GetName() + "                                                                         time: " + std::to_string(sum_time) + errors;
		//	wxString msg = "               CHECK ROUTE";
		wxString msg(str);
		wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		dc.SetFont(*g_pFontSmall);
		wxColour cl = wxColour(61, 61, 204, 255);
		dc.SetTextForeground(cl);
		dc.DrawText(msg, 10, 10);
	}
}

double Weather::calculate_speed_koef(ChartCanvas *cc, double h) {
	double N, D, L, delta;
	N = cc->GetShipN();
	D = cc->GetShipD();
	L = cc->GetShipL();
	delta = ((double)cc->GetShipDelta()) / 1000;
	if (N == 0 || D == 0 || L == 0 || delta == 0) {
		return 1;
	}

	double g, g1, g2, g3;
	g1 = std::pow((N / D), -1.14) - 2;
	g2 = std::pow((delta/(1.143 - 1.425 * N / D)), 4.7);
	g3 = 1.25 * std::exp(-1.48*(10 * h / L));
	g = g1 * g2 * g3;
	double k = std::exp(-g * (10 * h / L) * (10 * h / L));
	if (k > 1 || k <= 0) {
		k = 1;
	}
	return k;
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

void Weather::print_path_step(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, double lat, double lon) {
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
	green = 165;
	red = 255;
	blue = 0;

	wxColour hi_colour(red, green, blue, 255);
	//wxColour hi_colour = pen->GetColour();


	//  Highlite any selected point
	AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, radius,
		hi_colour, transparency);
}

void Weather::draw_calculate_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {


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

		find_fast_route(cc, dc, VP, box, pRouteDraw);
	}

	if (is_downloaded) {
		wxString msg = "\n               CALCULATE ROUTE" + std::to_string(2147483647 * 2);
		wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		dc.SetFont(*g_pFontSmall);
		wxColour cl = wxColour(61, 61, 204, 255);
		dc.SetTextForeground(cl);
		dc.DrawText(msg, 10, 10);
	}
}

bool Weather::is_in_weather_area(double lat1, double lon1) {
	if ((lat1 >= lat_min) && (lat1 <= lat_max) && (lon1 >= lon_min) && (lon1 <= lon_max)) {
		return true;
	}
	return false;
}

int Weather::get_lat_index(double lat) {
	lat = std::round(lat * 10) / 10;
	return (lat - lat_min) * 10;
}

int Weather::get_lon_index(double lon) {
	lon = std::round(lon * 10) / 10;
	return (lon - lon_min) * 10;
}

bool Weather::is_in_weather_grid(int lat_ind, int lon_ind) {
	if (lat_ind < 0 || lon_ind < 0) return false;
	if (lat_ind >= grid_data[0].second.size()) return false;
	if (lon_ind >= grid_data[0].second[0].size()) return false;

	return true;
}

std::pair<int, double> Weather::get_time_shift(double time) {
	int plus_index = time / (60 * 60 * 3);
	double shifted = time - (60 * 60 * 3) * plus_index;
	return { plus_index, shifted };
}


void Weather::find_fast_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route *route) {

	wxRoutePointListNode *node_start = route->pRoutePointList->GetFirst();
	RoutePoint *start = node_start->GetData();

	wxRoutePointListNode *node_finish = route->pRoutePointList->GetLast();
	RoutePoint *finish = node_finish->GetData();

	if (!is_in_weather_area(start->m_lat, start->m_lon)) return;
	if (!is_in_weather_area(finish->m_lat, finish->m_lon)) return;

	double v_nominal = cc->GetShipSpeed();

	double start_time_three_hours = cc->GetStartTimeThreeHours();
	std::vector<std::string> all_choices = GetChoicesDateTime();
	std::sort(all_choices.begin(), all_choices.end());

	std::string start_time = cc->GetStartTime();
	int ind_start_time = -1;
	for (int i = 0; i < grid_data.size(); i++) {
		if (grid_data[i].first == start_time) {
			ind_start_time = i;
			break;
		}
	}


	double start_grid_lat = std::round(start->m_lat * 10) / 10;
	double start_grid_lon = std::round(start->m_lon * 10) / 10;
	double finish_grid_lat = std::round(finish->m_lat * 10) / 10;
	double finish_grid_lon = std::round(finish->m_lon * 10) / 10;


	const double INF = 2147483647;
	std::vector<std::vector<double>> D;//lat --- lon
	//std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> q;
	std::priority_queue<std::pair<double, std::pair<int, int>>, std::vector<std::pair<double, std::pair<int, int>>>, std::greater<std::pair<double, std::pair<int, int>>>> q;

	int lat_size = (lat_max - lat_min) * 10 + 1;
	int lon_size = (lon_max - lon_min) * 10 + 1;

	for (int j = 0; j < lat_size; j++) {
		std::vector<double> temp;
		D.push_back(temp);
		D[D.size() - 1].resize(lon_size, INF);
	}

	int start_lat_ind = get_lat_index(start_grid_lat);
	int start_lon_ind = get_lon_index(start_grid_lon);
	int finish_lat_ind = get_lat_index(finish_grid_lat);
	int finish_lon_ind = get_lon_index(finish_grid_lon);
	D[start_lat_ind][start_lon_ind] = 0;
	
	q.push({ 0, {start_lat_ind, start_lon_ind} });

	while (!q.empty()) {
		std::pair<double, std::pair<int, int>>  p = q.top();
		q.pop();
		if (p.second.first == finish_lat_ind && p.second.second == finish_lon_ind) break;

		int now_lat_ind = p.second.first;
		int now_lon_ind = p.second.second;
		double now_weight = p.first;

		if (now_weight > D[now_lat_ind][now_lon_ind]) { continue; }

		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (i == 0 && j == 0) continue;
				int next_lat = now_lat_ind + i;
				int next_lon = now_lon_ind + j;
				if (!is_in_weather_grid(next_lat,next_lon)) continue;
				if (D[next_lat][next_lon] <= D[now_lat_ind][now_lon_ind]) continue;

				double way = std::sqrt((double)(i * i + j * j));

				/////
				//first half
				std::pair<int, double> times_for_now = get_time_shift(now_weight + start_time_three_hours);
				if (times_for_now.first + ind_start_time >= all_choices.size()) {
					continue;
				}
				std::string now_str_time = all_choices[times_for_now.first + ind_start_time];
				if (now_str_time == "" || now_str_time == "no data") continue;

				int ind_now_time = -1;
				for (int i = 0; i < all_choices.size(); i++) {
					if (all_choices[i] == now_str_time) {
						ind_now_time = i;
						break;
					}
				}
				PointWeatherData now_square = grid_data[ind_now_time].second[now_lat_ind][now_lon_ind];
				if (now_square.creation_time == "-1") continue;
				double sum_waves = now_square.wave_height + now_square.ripple_height;
				if (sum_waves >= ((double)cc->GetShipDangerHeight()) / 100 || !is_deep_enough(0, 0)) { //тут другие координаты, но все равно
					continue;
				}

				double v_first_half = v_nominal * calculate_speed_koef(cc, sum_waves);

				double time_half_way = (way / 2) / v_first_half;

				/////
				//second half
				std::pair<int, double> times_for_half_way = get_time_shift(now_weight + start_time_three_hours + time_half_way);
				if (times_for_half_way.first + ind_start_time >= all_choices.size()) {
					continue;
				}
				std::string half_way_str_time = all_choices[times_for_half_way.first + ind_start_time];
				if (half_way_str_time == "" || half_way_str_time == "no data") continue;

				int ind_half_time = -1;
				for (int i = 0; i < all_choices.size(); i++) {
					if (all_choices[i] == half_way_str_time) {
						ind_half_time = i;
						break;
					}
				}
				PointWeatherData half_square = grid_data[ind_half_time].second[next_lat][next_lon];
				if (half_square.creation_time == "-1") continue;
				double sum_waves_half = half_square.wave_height + half_square.ripple_height;
				if (sum_waves_half > ((double)cc->GetShipDangerHeight()) / 100 || !is_deep_enough(0, 0)) { //тут другие координаты, но все равно
					continue;
				}
				double v_second_half = v_nominal * calculate_speed_koef(cc, sum_waves_half);
				double time_second_half_way = (way / 2) / v_second_half;

				/////
				//finish check
				std::pair<int, double> times_for_whole_way = get_time_shift(now_weight + start_time_three_hours + time_half_way + time_second_half_way);
				if (times_for_whole_way.first + ind_start_time >= all_choices.size()) {
					continue;
				}
				std::string whole_way_str_time = all_choices[times_for_whole_way.first + ind_start_time];
				if (whole_way_str_time == "" || whole_way_str_time == "no data") continue;

				int ind_whole_time = -1;
				for (int i = 0; i < all_choices.size(); i++) {
					if (all_choices[i] == whole_way_str_time) {
						ind_whole_time = i;
						break;
					}
				}
				PointWeatherData whole_square = grid_data[ind_whole_time].second[next_lat][next_lon];
				if (whole_square.creation_time == "-1") continue;
				double sum_waves_whole = whole_square.wave_height + whole_square.ripple_height;
				if (sum_waves_whole > ((double)cc->GetShipDangerHeight()) / 100 || !is_deep_enough(0, 0)) { //тут другие координаты, но все равно
					continue;
				}
				
				///  все проверки пройдены, осталось обновить очередь и внести новые данные в массив

				double time_to_next = now_weight + time_half_way + time_second_half_way;
				if (time_to_next < D[next_lat][next_lon]) {
					D[next_lat][next_lon] = time_to_next;
					q.push({ time_to_next, {next_lat, next_lon} });
				}
			}
		}
	}

	if (D[finish_lat_ind][finish_lon_ind] < INF - 1) {
		std::vector<std::pair<int, int>> path;
		path.push_back({ finish_lat_ind, finish_lon_ind });
		int lat_ind_path = finish_lat_ind;
		int lon_ind_path = finish_lon_ind;

		while (lat_ind_path != start_lat_ind || lon_ind_path != start_lon_ind) {
			double min = INF;
			int lat_min_ind = -1;
			int lon_min_ind = -1;
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (i == 0 && j == 0) continue;
					int next_lat = lat_ind_path + i;
					int next_lon = lon_ind_path + j;
					if (!is_in_weather_grid(next_lat, next_lon)) continue;
					if (D[next_lat][next_lon] <= min) {
						min = D[next_lat][next_lon];
						lat_min_ind = next_lat;
						lon_min_ind = next_lon;
					}
				}
			}
			path.push_back({ lat_min_ind, lon_min_ind });
			lat_ind_path = lat_min_ind;
			lon_ind_path = lon_min_ind;
		}

		for (int i = 0; i < path.size(); i++) {
			//print_path_step(cc, dc,VP, box, path[i].first, path[i].second);
			print_path_step(cc, dc, VP, box, lat_min + ((double)path[i].first)/10, lon_min + ((double)path[i].second)/10);
		}

		if (is_downloaded) {
			wxString msg = "\n               CALCULATE ROUTE   " + std::to_string((D[finish_lat_ind][finish_lon_ind]));
			wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
			dc.SetFont(*g_pFontSmall);
			wxColour cl = wxColour(61, 61, 204, 255);
			dc.SetTextForeground(cl);
			dc.DrawText(msg, 10, 10);
		}
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