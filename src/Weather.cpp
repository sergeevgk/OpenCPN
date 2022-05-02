#include "wx/wxprec.h"

#include "Weather.h"
#include "weather_utils.h"
#include "weather_render_utilities.h"
#include "db_utils.h"
#include "routeman.h"
#include "navutil.h"
#include "chcanv.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#include <chrono>
#include <thread>
extern ocpnGLOptions g_GLOptions;
#endif
using namespace std;

extern Routeman		*g_pRouteMan;
extern float        g_ChartScaleFactorExp;
extern RouteList    *pRouteList;
extern TrackList    *pTrackList;
extern wxString		g_default_wp_icon;
extern WayPointman      *pWayPointMan;

const std::string TimeMeasureFileName = "C:\\Users\\gosha\\Documents\\GitHub\\openCPN_temp\\time.txt";

static void SaveKeyValueToFile(std::string fileName, std::string key, long long value) {
	std::ofstream myfile;
	myfile.open(TimeMeasureFileName, std::ofstream::app);
	myfile << key << " : " << value;
	myfile << std::endl;
	myfile.close();
}

static long long GetMsFromTimePoints(chrono::time_point<chrono::steady_clock> start, chrono::time_point<chrono::steady_clock> end)
{
	return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

#pragma optimize("", off)
Weather::Weather()
{
	db_context = new DbUtils::DbContext();
	//db_context->SeedDefaultData();
	auto end = chrono::steady_clock::now();
	refuge_place_vector = db_context->QuerySafePlaceData();
	ship_class_vector = db_context->QueryShipClasses();
	auto start = chrono::steady_clock::now();

	SaveKeyValueToFile(TimeMeasureFileName, "db_operations", GetMsFromTimePoints(end, start));
	
	//end = chrono::steady_clock::now();
	//Weather::download_weather_from_esimo();
	//start = chrono::steady_clock::now();
	//SaveKeyValueToFile(TimeMeasureFileName, "data_download", GetMsFromTimePoints(end, start));


	////data = get_all_weather_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\clean_data.csv");
	////data = get_all_weather_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\lena_test_data.csv");

	////get_all_weather_date_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\data_diff_days.csv");
	//get_all_weather_date_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\lena_test_data.csv");
	//
	////data = date_data[0].second;
	//if (data.size() > 0) {
	//	is_downloaded = true;
	//}
	//


	//dijstra test


	//data = get_all_weather_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\clean_data.csv");
	//data = get_all_weather_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\lena_test_data.csv");

	//get_all_weather_date_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\data_diff_days.csv");
	//get_all_weather_date_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\lena_test_data.csv");

	//get_all_weather_date_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\clean_data.csv");
	end = chrono::steady_clock::now();
	
	get_all_weather_date_data("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\test_data.csv");
	create_data_grid();

	//data = date_data[0].second;
	if (date_data[0].second.size() > 0) {
		is_downloaded = true;
	}
	start = chrono::steady_clock::now();
	SaveKeyValueToFile(TimeMeasureFileName, "data_preparation_no_download", GetMsFromTimePoints(end, start));

}
#pragma optimize("", on)

Weather::~Weather(void)
{

}

void Weather::Draw(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box)
{
	draw_refuge_places(cc, dc, VP, box);
	if (cc->GetDrawWaveHeightEnabled()) {
		auto end = chrono::steady_clock::now();
		draw_gradient(cc, dc, VP, box);
		auto start = chrono::steady_clock::now();
		SaveKeyValueToFile(TimeMeasureFileName, "draw_wave_gradient", GetMsFromTimePoints(end, start));
	}
	if (cc->GetCheckRouteEnabled()) {
		draw_check_route(cc, dc, VP, box);
	}
	if (cc->GetCalculateRouteEnabled()) {
		draw_calculate_route(cc, dc, VP, box);
	}
	if (cc->GetCheckOptimalRoute()) {
		draw_check_conflicts_on_route(cc, dc, VP, box, this->last_optimal_path);
	}
}

void Weather::draw_check_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {

	auto end = chrono::steady_clock::now();

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

		bool buildEscapeRouteOnConflict = false;
		double start_time_shift = 0;
		auto checkData = analyseRouteCheck(cc, dc, VP, box, pRouteDraw, start_time_shift, buildEscapeRouteOnConflict);
		
		// TODO: render only first conflict point before building rescue root
		checkData.Render(cc, dc, VP, box);
		
		// build rescue root from first conflict point
		if (buildEscapeRouteOnConflict && checkData.conflicts.size() > 0) {
			RoutePoint rp = RoutePoint(checkData.conflicts[0].latitude, checkData.conflicts[0].longitude, g_default_wp_icon, "conflict point", wxEmptyString, false);
			draw_find_refuge_roots(cc, dc, VP, box, rp, checkData.conflicts[0].shiftFromStartTime);
		}

		auto start = chrono::steady_clock::now();
		SaveKeyValueToFile(TimeMeasureFileName, "draw_check_route", GetMsFromTimePoints(end, start));
	}
}

WeatherUtils::RouteCheckData Weather::analyseRouteCheck(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route *route, double start_time_shift, bool build_rescue_root, bool draw_cone_lines) 
{
	wxRoutePointListNode *node = route->pRoutePointList->GetFirst();
	RoutePoint *prp2 = node->GetData();

	ChartBase *chart = cc->GetChartAtCursor();
	s57chart *s57ch = NULL;
	if (chart) {
		s57ch = dynamic_cast<s57chart*>(chart);
	}

	double sum_time = 0;//в часах
	double v_nominal = cc->GetShipSpeed();
	double v = -1;

	std::string start_time = cc->GetStartTime();
	
	double start_time_three_hours = cc->GetStartTimeThreeHours();
	double now_time_three_hours = start_time_three_hours / 60 / 60;//в часах
	int ind_start_time = -1;
	for (int i = 0; i < grid_data.size(); i++) {
		if (grid_data[i].first == start_time) {
			ind_start_time = i;
			break;
		}
	}
	std::vector<std::string> all_choices = GetChoicesDateTime();
	std::sort(all_choices.begin(), all_choices.end());
	std::string errors = "\n\n\n";
	
	ind_start_time = WeatherUtils::get_time_index(ind_start_time, all_choices, start_time_three_hours, start_time_shift);
	if (ind_start_time == -1) {
		return WeatherUtils::RouteCheckData();
	}
	std::string now_time = all_choices[ind_start_time];

	auto ship = WeatherUtils::ShipProperties(cc->GetShipDangerHeight(), cc->GetShipN(), cc->GetShipD(), cc->GetShipL(), cc->GetShipDelta(), v_nominal, cc->GetShipDraft());
	WeatherUtils::RouteCheckData routeCheckData(route, ship, ind_start_time);

	if (WeatherUtils::CheckGetCachedRouteCheckData(route_check_data, route, ship, ind_start_time, &routeCheckData))
	{
		return routeCheckData;
	};
	
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
			if (prev_grid_lat < 0 || prev_grid_lon < 0 || prev_grid_lat > lat_max || prev_grid_lon > lon_max)
				break;
			double sum_waves = -1;
			v = -1;
			
			check_land_collision(cc, dc, VP, box, prev_lat, prev_lon, s57ch, routeCheckData.conflicts, sum_time);

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
						if (sum_waves > ((double)cc->GetShipDangerHeight())/100) {
							std::string temp = "huge wave:" + std::to_string(sum_waves) + " in " + std::to_string(prev_lat) + " " + std::to_string(prev_lon);
							errors += temp + "\n";

							//WeatherUtils::print_zone(cc, dc, VP, box, prev_lat, prev_lon);
							routeCheckData.conflicts.push_back(WeatherUtils::ConflictData(prev_lat, prev_lon, 0, sum_time));
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
			double next_lat = norm_lat > 0 ? prev_lat + delta_lat : prev_lat - delta_lat;
			double next_lon = norm_lon > 0 ? prev_lon + delta_lon : prev_lon - delta_lon;

			wxPoint2DDouble start = wxPoint2DDouble(prev_lat, prev_lon);
			wxPoint2DDouble end = wxPoint2DDouble(next_lat, next_lon);
			double angle = WeatherUtils::calculate_cone_angle(v);
			wxPoint2DDouble end_rot1 = WeatherUtils::rotate_vector_around_first_point(start, end, angle);
			wxPoint2DDouble end_rot2 = WeatherUtils::rotate_vector_around_first_point(start, end, -angle);
			// рисуем конус до следующей точки, угол в зависимости от скорости (симметрично относительно пути)
			if (draw_cone_lines) {
				//WeatherUtils::draw_line_on_map(cc, dc, VP, box, prev_lat, prev_lon, end_rot1.m_x, end_rot1.m_y, wxColour(255, 0, 0, 255));
				//WeatherUtils::draw_line_on_map(cc, dc, VP, box, prev_lat, prev_lon, end_rot2.m_x, end_rot2.m_y, wxColour(255, 0, 0, 255));
				routeCheckData.checkCones.push_back(WeatherUtils::ConeData(
					{ prev_lat, prev_lon },
					{ end_rot1.m_x, end_rot1.m_y },
					{ end_rot2.m_x, end_rot2.m_y }));
			}
			// проверяем внутри него объекты на глубину
			
			check_depth_in_cone(s57ch, cc, dc, VP, box, start, end, routeCheckData.conflicts, sum_time);

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
	route_check_data.push_back(routeCheckData);

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
	return routeCheckData;
}

// TODO: try another way to check "part_of_route - earth" collision
// see PlugIn_GSHHS_CrossesLand function
void Weather::check_land_collision(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box,
	double lat, double lon, s57chart* chart, std::vector<WeatherUtils::ConflictData> &conflicts, double shift_time) {
	double select_radius = 1e-4;
	ListOfObjRazRules * map_objects = get_objects_at_lat_lon(lat, lon, select_radius, chart, &VP, MASK_AREA);
	if (is_land_area(map_objects, chart)) {
		//WeatherUtils::print_zone(cc, dc, VP, box, lat, lon, wxColour(255, 0, 0, 255));
		conflicts.push_back(WeatherUtils::ConflictData(lat, lon, 2, shift_time));
	}
}

void Weather::check_depth_in_cone(s57chart* chart, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP,
	const LLBBox &box, wxPoint2DDouble start, wxPoint2DDouble end, std::vector<WeatherUtils::ConflictData> &conflicts, double shift_time){
	// начальное значение радиуса для поиска объектов
	double draft = cc->GetShipDraft();
	float draft_in_ft = draft; // unit of measurement. - ft. // единицы измерения - футы
	double r = 1e-4;
	double angle = 0.017;
	double tan = std::tan(angle);
	wxPoint2DDouble step = WeatherUtils::step_for_way(start, end, r);
	// цикл с некоторым шагом по пути движения
	for (wxPoint2DDouble point = start - step; point.GetDistance(end) >= r;  point += step) {
		// получение объектов в радиусе
		r += tan * step.GetDistance(wxPoint2DDouble(0, 0)); // тангенс угла
		step = step * (1.0f + tan);
		ListOfObjRazRules* select_objects = get_objects_at_lat_lon(point.m_x, point.m_y, r, chart, &VP, MASK_ALL-MASK_AREA);
		ListOfObjRazRules* select_areas = get_objects_at_lat_lon(point.m_x, point.m_y, r, chart, &VP, MASK_AREA);
		if (select_objects->Number() == 0 && select_areas->Number() == 0)
			continue;

		if (select_objects->Number() != 0 && !is_deep_enough(select_objects, chart, draft_in_ft)) {
			//WeatherUtils::print_zone(cc, dc, VP, box, point.m_x, point.m_y, wxColour(255, 0, 255, 255));
			conflicts.push_back(WeatherUtils::ConflictData(point.m_x, point.m_y, 1, shift_time));
		}

		if (select_areas->Number() != 0 && !is_deep_enough_area(select_areas, chart, draft_in_ft)) {
			//WeatherUtils::print_zone(cc, dc, VP, box, point.m_x, point.m_y, wxColour(255, 0, 255, 255));
			conflicts.push_back(WeatherUtils::ConflictData(point.m_x, point.m_y, 1, shift_time));
		}
	}
	return;
}

bool Weather::is_depth_in_cone_enough(s57chart* chart, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, wxPoint2DDouble start, wxPoint2DDouble end) {
	// начальное значение радиуса для поиска объектов
	double draft = cc->GetShipDraft();
	float draft_in_ft = draft; // unit of measurement. - ft. // единицы измерения - футы
	double r = 1e-4;
	double angle = 0.017;
	double tan = std::tan(angle);
	wxPoint2DDouble step = WeatherUtils::step_for_way(start, end, r);
	// цикл с некоторым шагом по пути движения
	for (wxPoint2DDouble point = start - step; point.GetDistance(end) >= r; point += step) {
		// получение объектов в радиусе
		r += tan * step.GetDistance(wxPoint2DDouble(0, 0)); // тангенс угла
		step = step * (1.0f + tan);
		ListOfObjRazRules* select_objects = get_objects_at_lat_lon(point.m_x, point.m_y, r, chart, &VP, MASK_ALL - MASK_AREA);
		ListOfObjRazRules* select_areas = get_objects_at_lat_lon(point.m_x, point.m_y, r, chart, &VP, MASK_AREA);
		if (select_objects->Number() == 0 && select_areas->Number() == 0)
			continue;

		if (select_objects->Number() != 0 && !is_deep_enough(select_objects, chart, draft_in_ft)) {
			return false;
		}

		if (select_areas->Number() != 0 && !is_deep_enough_area(select_areas, chart, draft_in_ft)) {
			return false;
		}
	}
	return true;
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
	if ((N / D) > 1.143 / 1.425) {
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

bool Weather::is_land_area(ListOfObjRazRules *list, s57chart *chart) {
	std::string land_area_attribute = "LNDARE";
	if (list == NULL) {
		return false;
	}
	if (list->GetCount() == 0) {
		return false;
	}
	for (ListOfObjRazRules::Node *node = list->GetLast(); node; node = node->GetPrevious()) {
		ObjRazRules *current = node->GetData();
		if (current == NULL || current->obj == NULL) continue;
		if (current->obj->Primitive_type == GEO_META) continue;
		if (current->obj->Primitive_type == GEO_PRIM) continue;
		bool isLight = !strcmp(current->obj->FeatureName, "LIGHTS");
		if (isLight) continue;

		wxString curAttrName;
		std::string featureName = std::string(current->obj->FeatureName);
		if (featureName.find(land_area_attribute) != std::string::npos)
			return true;
		if (current->obj->att_array) {
			char *curr_att = current->obj->att_array;
			int attrCounter = 0;
			while (attrCounter < current->obj->n_attr) {
				curAttrName = wxString(curr_att, wxConvUTF8, 6);
				wxString value = chart->GetObjectAttributeValueAsString(current->obj, attrCounter, curAttrName);
				if (value.Contains(land_area_attribute))
					return true;
				attrCounter++;
				curr_att += 6;
			}
		}
	}
	return false;
}

ListOfObjRazRules * Weather::get_objects_at_lat_lon(double lat, double lon, double select_rad, s57chart* chart, ViewPort *VP, int mask) {
	//float select_radius = select_rad_pix;// / (VP->view_scale_ppm * 1852 * 60);
	ListOfObjRazRules *list = NULL;
	wxString allValues;
	if (chart) {
		list = chart->GetObjRuleListAtLatLon(lat, lon, select_rad, VP, mask);
	}
	/*if (list != NULL) {
		wxString allValues;
		allValues.Append(std::to_string(lat));
		allValues.Append(", ");
		allValues.Append(std::to_string(lon));
		allValues.Append(" count ");
		allValues.Append(std::to_string((int)list->GetCount()));
		allValues.Append("\n");
		std::ofstream myfile;
		std::string lat_lon = std::to_string(lat) + std::to_string(lon);
		myfile.open("C:\\Users\\gosha\\Documents\\GitHub\\openCPN_temp\\output" + lat_lon + ".txt", std::ofstream::app);
		myfile << allValues.ToStdString();
		myfile.close();
	}*/
	return list;
}

bool Weather::print_objects_values_to_file(ListOfObjRazRules* list, s57chart* chart) {
	if (list == NULL) {
		return false;
	}
	if (list->GetCount() == 0) {
		return false;
	}
	wxString allValues;
	for (ListOfObjRazRules::Node *node = list->GetLast(); node; node = node->GetPrevious()) {
		ObjRazRules *current = node->GetData();
		if (current == NULL || current->obj == NULL) continue;
		if (current->obj->Primitive_type == GEO_META) continue;
		if (current->obj->Primitive_type == GEO_PRIM) continue;
		bool isLight = !strcmp(current->obj->FeatureName, "LIGHTS");
		if (isLight) continue;

		wxString curAttrName;
		std::string featureName = std::string(current->obj->FeatureName);
		allValues.Append(featureName);
		allValues.Append(" == ");
		if (current->obj->att_array) {
			char *curr_att = current->obj->att_array;
			int attrCounter = 0;
			while (attrCounter < current->obj->n_attr) {
				curAttrName = wxString(curr_att, wxConvUTF8, 6);
				wxString value = chart->GetObjectAttributeValueAsString(current->obj, attrCounter, curAttrName);
				allValues.Append(curAttrName);
				allValues.Append(" : ");
				allValues.Append(value);
				allValues.Append(", ");
				attrCounter++;
				curr_att += 6;
			}
		}
		allValues.Append("\n");
	}
	std::ofstream myfile;
	myfile.open("C:\\Users\\gosha\\Documents\\GitHub\\openCPN_temp\\output.txt", std::ofstream::app);
	myfile << allValues.ToStdString();
	myfile.close();
	return false;
}

// @param draft - осадка судна
bool Weather::is_deep_enough(ListOfObjRazRules* list, s57chart* chart, float draft) {
	if (list == NULL) {
		return false;
	}
	if (list->GetCount() == 0) {
		return false;
	}
	// проверка атрибутов на наличие глубины (скалы/ области мелей / другие одиночные объекты с глубиной)
	// VALDCO - для depth contour, VALSOU - для остальных
	for (ListOfObjRazRules::Node *node = list->GetLast(); node; node = node->GetPrevious()) {
		ObjRazRules *current = node->GetData();
		if (current == NULL || current->obj == NULL) continue;
		if (current->obj->Primitive_type == GEO_META) continue;
		if (current->obj->Primitive_type == GEO_PRIM) continue;
		// пока не знаю, надо ли их исключать
		bool isLight = !strcmp(current->obj->FeatureName, "LIGHTS");
		if (isLight) continue;

		wxString curAttrName;
		std::string featureName = std::string(current->obj->FeatureName);
		if (current->obj->att_array) {
			char *curr_att = current->obj->att_array;
			int attrCounter = 0;
			while (attrCounter < current->obj->n_attr) {
				curAttrName = wxString(curr_att, wxConvUTF8, 6);
				if (curAttrName != "VALSOU") {
					attrCounter++;
					curr_att += 6;
					continue;
				}
				wxString value = chart->GetObjectAttributeValueAsString(current->obj, attrCounter, curAttrName);
				float depth_value = atof(value.c_str());
				if (depth_value < draft) {
					return false;
				}
				attrCounter++;
				curr_att += 6;
			}
		}
	}
	return true;
}

// @param draft - осадка судна
bool Weather::is_deep_enough_area(ListOfObjRazRules* list, s57chart* chart, float draft) {
	if (list == NULL) {
		return false;
	}
	if (list->GetCount() == 0) {
		return false;
	}
	// проверка атрибутов на наличие глубины (скалы/ области мелей / другие одиночные объекты с глубиной)
	// VALDCO - для depth contour, VALSOU - для остальных
	for (ListOfObjRazRules::Node *node = list->GetLast(); node; node = node->GetPrevious()) {
		ObjRazRules *current = node->GetData();
		if (current == NULL || current->obj == NULL) continue;
		int isDepthArea = strcmp(current->obj->FeatureName, "DEPARE"); // только области глубины
		if (isDepthArea != 0) 
			continue;
		wxString curAttrName;
		std::string featureName = std::string(current->obj->FeatureName);
		if (current->obj->att_array) {
			char *curr_att = current->obj->att_array;
			int attrCounter = 0;
			while (attrCounter < current->obj->n_attr) {
				curAttrName = wxString(curr_att, wxConvUTF8, 6);
				if (curAttrName != "DRVAL1") {
					attrCounter++;
					curr_att += 6;
					continue;
				}
				wxString value = chart->GetObjectAttributeValueAsString(current->obj, attrCounter, curAttrName);
				float depth_value = atof(value.c_str());
				if (depth_value < draft) {
					return false;
				}
				attrCounter++;
				curr_att += 6;
			}
		}
	}
	return true;
}

bool Weather::is_same_colour(wxColour first, wxColour second) {
	return (first.Alpha() == second.Alpha()) &&
		(first.Red() == second.Red()) &&
		(first.Blue() == second.Blue()) &&
		(first.Green() == second.Green());
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

void Weather::highlight_considered_grid(std::vector<std::vector<int>> &grid, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
	 auto route_color = wxColour(100, 0, 0, 255);
	 auto zone_color = wxColour(200, 0, 0, 255);
	 
	 for (int i = 0; i < grid.size(); i++) {
		 for (int j = 0; j < grid[0].size(); j++) {
			 double lat = WeatherUtils::get_coordinate_from_index(i, lat_min);
			 double lon = WeatherUtils::get_coordinate_from_index(j, lon_min);
			 if (grid[i][j] == 0) {
				 WeatherUtils::print_zone(cc, dc, VP, box, lat, lon, zone_color);
			 }
			 if (grid[i][j] == 1) {
				 WeatherUtils::print_zone(cc, dc, VP, box, lat, lon, zone_color);
			 }
		 }
	 }
}

int Weather::get_next_nearest_refuge_place_index(std::vector<WeatherUtils::RefugePlace> refuge_place_vector, RoutePoint position, int current_nearest_index = -1)
{
	int min_distance_index = -1;
	double min_distance = INFINITY - 1;
	wxPoint2DDouble pos = wxPoint2DDouble(position.m_lat, position.m_lon);
	for (int i = 0; i < refuge_place_vector.size(); i++) {
		if (i == current_nearest_index)
			continue;
		wxPoint2DDouble place = wxPoint2DDouble(refuge_place_vector[i].latitude, refuge_place_vector[i].longitude);
		double distance = WeatherUtils::get_distance(pos, place);
		if (distance < min_distance) {
			min_distance_index = i;
			min_distance = distance;
		}
	}
	return min_distance_index;
}

void Weather::draw_find_refuge_roots(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, RoutePoint current_position, double rescue_start_time) {
	Route *pRoute = new Route();
	pRoute->AddPoint(&current_position);
	
	// find next nearest refuge place
	int nearest_index = get_next_nearest_refuge_place_index(refuge_place_vector, current_position);
	auto refuge = refuge_place_vector[nearest_index];

	RoutePoint p = RoutePoint(refuge.latitude, refuge.longitude, g_default_wp_icon, refuge.name, wxEmptyString, false);
	pRoute->AddPoint(&p);

	auto considered_zone_builder = WeatherUtils::ConsideredZoneBuilder(ZONE_WIDTH_DEFAULT, new double[4]{ lat_min, lat_max, lon_min, lon_max });

	// don't save to last_optimal_path
	auto routeBuildData = find_fast_route(cc, dc, VP, box, pRoute, considered_zone_builder, rescue_start_time);

	// if route was not found - increase width of considered zone x2 (once)
	if (routeBuildData.optimal_path.size() == 0) {
		considered_zone_builder.increase_zone_width(2);
		// don't save to last_optimal_path
		routeBuildData = find_fast_route(cc, dc, VP, box, pRoute, considered_zone_builder, rescue_start_time);

		// if not found - try get another (next by distance) refuge place
		// TODO - later

		// if still not found - build simple route with marked conflicts
		draw_simple_refuge_root_with_conflicts(cc, dc, VP, box, pRoute, rescue_start_time);
	}

	pRoute->RemovePoint(&p);
}

// this method is draft and currently not used
void Weather::draw_simple_refuge_root_with_conflicts(ChartCanvas* cc, ocpnDC& dc, ViewPort& VP, const LLBBox& box, Route* route, vector<vector<int>> considered_zone_grid, double rescue_start_time)
{
	wxRoutePointListNode *node_start = route->pRoutePointList->GetFirst();
	RoutePoint *start = node_start->GetData();
	wxRoutePointListNode *node_finish = route->pRoutePointList->GetLast();
	RoutePoint *finish = node_finish->GetData();

	double lat_start = std::round(start->m_lat * 10) / 10;
	double lon_start = std::round(start->m_lon * 10) / 10;
	double lat_finish = std::round(finish->m_lat * 10) / 10;
	double lon_finish = std::round(finish->m_lon * 10) / 10;
	int start_lat_idx = WeatherUtils::get_coordinate_index(lat_start, lat_min);
	int start_lon_idx = WeatherUtils::get_coordinate_index(lon_start, lon_min);
	int finish_lat_idx = WeatherUtils::get_coordinate_index(lat_finish, lat_min);
	int finish_lon_idx = WeatherUtils::get_coordinate_index(lon_finish, lon_min);

	double v_nominal = cc->GetShipSpeed();

	std::string start_time = cc->GetStartTime();
	int ind_start_time = -1;
	for (int i = 0; i < grid_data.size(); i++) {
		if (grid_data[i].first == start_time) {
			ind_start_time = i;
			break;
		}
	}
	double start_time_three_hours = cc->GetStartTimeThreeHours();
	std::vector<std::string> all_choices = GetChoicesDateTime();
	std::sort(all_choices.begin(), all_choices.end());

	ind_start_time = WeatherUtils::get_time_index(ind_start_time, all_choices, start_time_three_hours, rescue_start_time);
	if (ind_start_time == -1) {
		return;
	}
	int ind_now_time = ind_start_time;
	double elapsed_time = 0;
	double time_to_next = 0;
	// trace the grid by "0" cells 
	for (int i = start_lat_idx, j = start_lon_idx; ; i < finish_lat_idx && j < finish_lon_idx) {
		double lat = WeatherUtils::get_coordinate_from_index(i, lat_min);
		double lon = WeatherUtils::get_coordinate_from_index(j, lon_min);
		PointWeatherData now_square = grid_data[ind_now_time].second[i][j];
		/*if (now_square.creation_time == "-1") 
			continue;*/

		double sum_waves = now_square.wave_height + now_square.ripple_height;
		//checking weather conflicts 
		if (sum_waves >= ((double)cc->GetShipDangerHeight()) / 100) {
			// draw square of purple (conflict) color
			WeatherUtils::print_zone(cc, dc, VP, box, lat, lon);
		}
		//checking static conflicts on sections
		else if (false) {
			// draw square of purple (conflict) color
			WeatherUtils::print_zone(cc, dc, VP, box, lat, lon);
		}
		else {
			// draw square of orange (safe route) color
			print_path_step(cc, dc, VP, box, lat, lon);
		}
		double v_first_half = v_nominal * calculate_speed_koef(cc, sum_waves);
		ind_now_time = WeatherUtils::get_time_index(ind_start_time, all_choices, start_time_three_hours, elapsed_time);
		if (ind_now_time == -1) {
			return;
		}
		double way = 1;
		double time_half_way = (way / 2) / v_first_half;

		// elapsed_time += time_to_next
	}
}

void Weather::draw_simple_refuge_root_with_conflicts(ChartCanvas * cc, ocpnDC & dc, ViewPort & VP, const LLBBox & box, Route * route, double rescue_start_time)
{
	wxRoutePointListNode *node_start = route->pRoutePointList->GetFirst();
	RoutePoint *start = node_start->GetData();
	wxRoutePointListNode *node_finish = route->pRoutePointList->GetLast();
	RoutePoint *finish = node_finish->GetData();
	// draw line btwn route endpoints
	WeatherUtils::draw_line_on_map(cc, dc, VP, box, start->m_lat, start->m_lon, finish->m_lat, finish->m_lon, wxColour(0, 255, 0, 255));

	// call analyseCheckRoute();
	analyseRouteCheck(cc, dc, VP, box, route, rescue_start_time, false, false);
}

void Weather::draw_calculate_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
	if (cc->GetShipSpeed() <= 0) { return; }
	if (cc->GetStartTimeThreeHours() >= 3 * 60 * 60) return;
	if ((cc->GetStartTime() == "no data") || cc->GetStartTime() == "") return;
	auto end = chrono::steady_clock::now();

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
	
		auto considered_zone_builder = WeatherUtils::ConsideredZoneBuilder(ZONE_WIDTH_DEFAULT, new double[4]{ lat_min, lat_max, lon_min, lon_max });
		// save to last_optimal_path in order to use it in checking optimal route
		auto routeBuildData = find_fast_route(cc, dc, VP, box, pRouteDraw, considered_zone_builder);
		last_optimal_path = routeBuildData.optimal_path;

		routeBuildData.Render(cc, dc, VP, box);
		auto start = chrono::steady_clock::now();
		SaveKeyValueToFile(TimeMeasureFileName, "draw_calculate_optimal_route", GetMsFromTimePoints(end, start));

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

bool Weather::is_in_weather_grid(int lat_ind, int lon_ind) {
	if (lat_ind < 0 || lon_ind < 0) return false;
	if (lat_ind >= grid_data[0].second.size()) return false;
	if (lon_ind >= grid_data[0].second[0].size()) return false;

	return true;
}

//
// Returns an optimal path of grid cells indices if such exists for particular route.
//
WeatherUtils::RouteBuildData Weather::find_fast_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, Route *route, WeatherUtils::ConsideredZoneBuilder zone_builder, double start_time_shift) {
	ChartBase *chart = cc->GetChartAtCursor();
	s57chart *s57ch = NULL;
	if (chart) {
		s57ch = dynamic_cast<s57chart*>(chart);
	}
	auto empty_result = WeatherUtils::RouteBuildData(lat_min, lon_min);
	wxRoutePointListNode *node_start = route->pRoutePointList->GetFirst();
	RoutePoint *start = node_start->GetData();

	wxRoutePointListNode *node_finish = route->pRoutePointList->GetLast();
	RoutePoint *finish = node_finish->GetData();

	if (!is_in_weather_area(start->m_lat, start->m_lon)) return empty_result;
	if (!is_in_weather_area(finish->m_lat, finish->m_lon)) return empty_result;

	double v_nominal = cc->GetShipSpeed();

	std::string start_time = cc->GetStartTime();
	int ind_start_time = -1;
	for (int i = 0; i < grid_data.size(); i++) {
		if (grid_data[i].first == start_time) {
			ind_start_time = i;
			break;
		}
	}
	double start_time_three_hours = cc->GetStartTimeThreeHours();
	std::vector<std::string> all_choices = GetChoicesDateTime();
	std::sort(all_choices.begin(), all_choices.end());

	ind_start_time = WeatherUtils::get_time_index(ind_start_time, all_choices, start_time_three_hours, start_time_shift);
	if (ind_start_time == -1) {
		return empty_result;
	}

	auto ship = WeatherUtils::ShipProperties(cc->GetShipDangerHeight(), cc->GetShipN(), cc->GetShipD(), cc->GetShipL(), cc->GetShipDelta(), v_nominal, cc->GetShipDraft());
	WeatherUtils::RouteBuildData routeBuildData(lat_min, lon_min, start, finish, ship, ind_start_time);

	if (WeatherUtils::CheckGetCachedRouteBuildData(route_build_data, start, finish, ship, ind_start_time, &routeBuildData))
	{
		return routeBuildData;
	};

	auto considered_zone = zone_builder.BuildConsideredZoneFromRoute(route);

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

	int start_lat_ind = WeatherUtils::get_coordinate_index(start_grid_lat, this->lat_min);
	int start_lon_ind = WeatherUtils::get_coordinate_index(start_grid_lon, this->lon_min);
	int finish_lat_ind = WeatherUtils::get_coordinate_index(finish_grid_lat, this->lat_min);
	int finish_lon_ind = WeatherUtils::get_coordinate_index(finish_grid_lon, this->lon_min);
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
				int next_lat_ind = now_lat_ind + i;
				int next_lon_ind = now_lon_ind + j;
				if (!is_in_weather_grid(next_lat_ind,next_lon_ind)) continue;
				if (D[next_lat_ind][next_lon_ind] <= D[now_lat_ind][now_lon_ind]) continue;
				if (considered_zone[next_lat_ind][next_lon_ind] == -1) continue;
				double way = std::sqrt((double)(i * i + j * j));

				/////
				//first half
				std::pair<int, double> times_for_now = WeatherUtils::get_time_shift(now_weight + start_time_three_hours);
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
				if (sum_waves >= ((double)cc->GetShipDangerHeight()) / 100 ){
					continue;
				}
				double v_first_half = v_nominal * calculate_speed_koef(cc, sum_waves);


				//TODO проверка на глубину. Будет осуществляться по кратчайшему пути, без учета прохода по граням и тд.
				// возможно придется поделить путь пополам и проверить по двум половинам пути с учетом разной скорости для раствора конусов
				// получить координаты start и finish из индексов
				double now_lat = WeatherUtils::get_coordinate_from_index(now_lat_ind, this->lat_min);
				double now_lon = WeatherUtils::get_coordinate_from_index(now_lon_ind, this->lon_min);
				double next_lat = WeatherUtils::get_coordinate_from_index(next_lat_ind, this->lat_min);
				double next_lon = WeatherUtils::get_coordinate_from_index(next_lon_ind, this->lon_min);
				
				// вызвать is_depth_in_cone_enough() с проверкой результата на истину
				if (!is_depth_in_cone_enough(s57ch, cc, dc, VP, box, wxPoint2DDouble(now_lat, now_lon), wxPoint2DDouble(next_lat, next_lon))) {
					continue;
				}

				double time_half_way = (way / 2) / v_first_half;

				/////
				//second half
				std::pair<int, double> times_for_half_way = WeatherUtils::get_time_shift(now_weight + start_time_three_hours + time_half_way);
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
				PointWeatherData half_square = grid_data[ind_half_time].second[next_lat_ind][next_lon_ind];
				if (half_square.creation_time == "-1") continue;
				double sum_waves_half = half_square.wave_height + half_square.ripple_height;
				if (sum_waves_half > ((double)cc->GetShipDangerHeight()) / 100){
					continue;
				}
				
				double v_second_half = v_nominal * calculate_speed_koef(cc, sum_waves_half);
				double time_second_half_way = (way / 2) / v_second_half;

				/////
				//finish check
				std::pair<int, double> times_for_whole_way = WeatherUtils::get_time_shift(now_weight + start_time_three_hours + time_half_way + time_second_half_way);
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
				PointWeatherData whole_square = grid_data[ind_whole_time].second[next_lat_ind][next_lon_ind];
				if (whole_square.creation_time == "-1") continue;
				double sum_waves_whole = whole_square.wave_height + whole_square.ripple_height;
				if (sum_waves_whole > ((double)cc->GetShipDangerHeight()) / 100){
					continue;
				}
				
				///  все проверки пройдены, осталось обновить очередь и внести новые данные в массив

				double time_to_next = now_weight + time_half_way + time_second_half_way;
				if (time_to_next < D[next_lat_ind][next_lon_ind]) {
					D[next_lat_ind][next_lon_ind] = time_to_next;
					q.push({ time_to_next, {next_lat_ind, next_lon_ind} });
				}
			}
		}
	}
	if (D[finish_lat_ind][finish_lon_ind] < INF - 1) {
		vector<pair<double, pair<int, int>>> path;
		pair<double, pair<int, int>> time_coordinates_pair = {
			D[finish_lat_ind][finish_lon_ind],
			{ finish_lat_ind, finish_lon_ind } 
		};
		path.push_back(time_coordinates_pair);
		int lat_ind_path = finish_lat_ind;
		int lon_ind_path = finish_lon_ind;

		while (lat_ind_path != start_lat_ind || lon_ind_path != start_lon_ind) {
			double min = INF;
			int lat_min_ind = -1;
			int lon_min_ind = -1;
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (i == 0 && j == 0) continue;
					int next_lat_ind = lat_ind_path + i;
					int next_lon_ind = lon_ind_path + j;
					if (!is_in_weather_grid(next_lat_ind, next_lon_ind)) continue;
					if (D[next_lat_ind][next_lon_ind] <= min) {
						min = D[next_lat_ind][next_lon_ind];
						lat_min_ind = next_lat_ind;
						lon_min_ind = next_lon_ind;
					}
				}
			}
			pair<double, pair<int, int>> time_coordinates_pair_next = {
				D[lat_min_ind][lon_min_ind],
				{ lat_min_ind, lon_min_ind }
			};
			path.push_back(time_coordinates_pair_next);
			lat_ind_path = lat_min_ind;
			lon_ind_path = lon_min_ind;
		}
		routeBuildData.optimal_path = path;
		routeBuildData.considered_zone_grid = considered_zone;
		route_build_data.push_back(routeBuildData);
		return routeBuildData;
	}
	return empty_result;
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

	auto min_lat_ind = WeatherUtils::get_coordinate_index(box.GetMinLat(), lat_min);
	auto max_lat_ind = WeatherUtils::get_coordinate_index(box.GetMaxLat(), lat_min);
	auto min_lon_ind = WeatherUtils::get_coordinate_index(box.GetMinLon(), lon_min);
	auto max_lon_ind = WeatherUtils::get_coordinate_index(box.GetMaxLon(), lon_min);
	if (min_lat_ind < 0) min_lat_ind = 0;
	if (min_lon_ind < 0) min_lon_ind = 0;
	if (max_lat_ind > now_data.size()) max_lat_ind = now_data.size();
	//if (max_lon_ind > now_data[0].size()) max_lon_ind = now_data[i].size();

	for (int i = min_lat_ind; i < max_lat_ind; i++) {
		for (int j = min_lon_ind; j < max_lon_ind; j++) {
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

void Weather::draw_refuge_places(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {

	std::vector<std::vector<PointWeatherData>> now_data;

	for (WeatherUtils::RefugePlace refuge : refuge_place_vector){
			wxPoint r;
			wxRect hilitebox;
			double lat, lon;
			
			lat = refuge.latitude;
			lon = refuge.longitude;

			cc->GetCanvasPointPix(lat, lon, &r);

			wxPen *pen;
			pen = g_pRouteMan->GetRoutePointPen();

			int sx2 = 2;
			int sy2 = 2;

			wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);           // the bitmap extents

			hilitebox = r1;
			hilitebox.x -= r.x;
			hilitebox.y -= r.y;

			hilitebox.width *= g_ChartScaleFactorExp;
			hilitebox.height *= g_ChartScaleFactorExp;

			hilitebox.width *= VP.view_scale_ppm;
			hilitebox.height *= VP.view_scale_ppm;

			float radius;
			hilitebox.Inflate(15);
			radius = 15.0f;

			unsigned char transparency = 150;

			unsigned char red, green, blue;
			red = 50;
			green = 150;
			blue = 0;
			wxColour hi_colour(red, green, blue, 255);
			//wxColour hi_colour = pen->GetColour();

			//  Highlite any selected point
			AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, radius,
				hi_colour, transparency);
	}
}

//
// summary: this method draws conflict which was found on provided 'optimal route'.
// It is called on Draw action when checkbox activated
// params:
// route - optimal route which consists of pair<lat_index, lon_index> of weather grid.
//
void Weather::draw_check_conflicts_on_route(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, vector<pair<double, pair<int, int>>> route) {
	// call foreach cell in route check_conflicts_in_weather_grid_cell
	ChartBase *chart = cc->GetChartAtCursor();
	s57chart *s57ch = NULL;
	if (chart) {
		s57ch = dynamic_cast<s57chart*>(chart);
	}
	double sum_time = 0;
	for (auto step : route) {
		sum_time += step.first;
		bool check = check_conflicts_in_weather_grid_cell(s57ch, cc, dc, VP, box, step.second.first, step.second.second);
		if (check) {
			double lat = WeatherUtils::get_coordinate_from_index(step.second.first, lat_min);
			double lon = WeatherUtils::get_coordinate_from_index(step.second.second, lon_min);
			// draw conflict area
			WeatherUtils::print_zone(cc, dc, VP, box, lat, lon);
			//draw_find routes to refuge places (at least one)
			auto pos = RoutePoint(lat, lon, g_default_wp_icon, "conflict_position", wxEmptyString, false);
			draw_find_refuge_roots(cc, dc, VP, box, pos, sum_time);
		}
	}
}

bool Weather::check_conflicts_in_weather_grid_cell(s57chart* chart, ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box, int lat_ind, int lon_ind) {
	double lat = WeatherUtils::get_coordinate_from_index(lat_ind, lat_min);
	double lon = WeatherUtils::get_coordinate_from_index(lon_ind, lon_min);
	double radius = 5556; // 3 sea miles im meters
	// get objects in radius = 1/2 cell width
	auto wayPoint = pWayPointMan->GetNearbyWaypoint(lat, lon, radius);
	if (wayPoint == nullptr)
		return false;
	// find object with icon "Hazard-Danger" and text starting with "weather_conflict"
	// return true if found
	auto mark_name = wayPoint->GetName();
	auto prefix = wxString("weather_conflict");
	if (mark_name.StartsWith(prefix)) {
		return true;
	}
	return false;
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
		fp = fopen("C:\\Users\\gosha\\Documents\\GitHub\\OpenCPN\\Weather\\test_data.csv", "wb");
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