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
	double lat_min = find_min_latitude();
	double lat_max = find_max_latitude();
	double lon_min = find_min_longitude();
	double lon_max = find_max_longitude();

	int lat_size = (lat_max - lat_min) * 10 + 1;
	int lon_size = (lon_max - lon_min) * 10 + 1;

	
	for (int i = 0; i < date_data.size(); i++) {
		std::vector<std::vector<PointWeatherData>> t;//lat --- lon
		for (int j = 0; j < lat_size; j++) {
			std::vector<PointWeatherData> temp;
			t.push_back(temp);
		}

		for (int ind = 0; ind < date_data[i].second.size(); ind++) {
			int lat_ind = (date_data[i].second[ind].latitude - lat_min) * 10;
			int lon_ind = (date_data[i].second[ind].longitude - lon_min) * 10;
			if (lat_ind >= lat_size) continue;
			if (t[lat_ind].size() == 0) {
				t[lat_ind].resize(lon_size);
			}
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