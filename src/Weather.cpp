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

	Weather::download_weather_from_esimo();
	//data = get_all_weather_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\clean_data.csv");
	data = get_all_weather_data("C:\\Users\\Admin\\Sources\\OpenCPN\\Weather\\lena_test_data.csv");
	
	
	if (data.size() > 0) {
		is_downloaded = true;
	}
	
}

Weather::~Weather(void)
{

}

void Weather::Draw(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box)
{
	if (draw_downloaded) {
		for (int i = 0; i < data.size(); i++) {
			wxPoint r;
			wxRect hilitebox;

			double lat, lon;
			lat = data[i].latitude;
			lon = data[i].longitude;

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

			wxColour hi_colour = pen->GetColour();
			unsigned char transparency = 100;

			//  Highlite any selected point
			AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width, hilitebox.height, radius,
				hi_colour, transparency);

			// написать текст в верхнем левом углу (отлияный способ быстро понять, что что-то работает)
			if (is_downloaded) {
				wxString msg = "I was here";
				wxFont* g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
				dc.SetFont(*g_pFontSmall);
				wxColour cl = wxColour(61, 61, 204, 255);
				dc.SetTextForeground(cl);
				dc.DrawText(msg, 10, 10);
			}
		}
	}
	else {
		draw_gradient(cc, dc, VP, box);
	}
}

void Weather::draw_gradient(ChartCanvas *cc, ocpnDC& dc, ViewPort &VP, const LLBBox &box) {
	for (int i = 0; i < data.size(); i++) {
		wxPoint r;
		wxRect hilitebox;

		double lat, lon;
		lat = data[i].latitude;
		lon = data[i].longitude;

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
			now = data[i].wave_height;
			min_value = min_value_wave;
			max_value = max_value_wave;
			
		}
		else if (mode == RIPPLE_HEIGHT) {
			now = data[i].ripple_height;
			min_value = min_value_ripple;
			max_value = max_value_ripple;
		}
		else {
			now = data[i].wave_height + data[i].ripple_height;
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
			red = (now - danger) * 255 / (range);
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
	if (data.size() == 0) {
		return -1;
	}

	double max = data[0].wave_height;

	for (int i = 0; i < data.size(); i++) {
		double temp = data[i].wave_height;
		if (temp > max) {
			max = temp;
		}
	}
	return max;
}

double Weather::find_min_wave_height() {
	if (data.size() == 0) {
		return -1;
	}

	double min = data[0].wave_height;

	for (int i = 0; i < data.size(); i++) {
		double temp = data[i].wave_height;
		if (temp < min) {
			min = temp;
		}
	}
	return min;
}

double Weather::find_max_ripple_height() {
	if (data.size() == 0) {
		return -1;
	}

	double max = data[0].ripple_height;

	for (int i = 0; i < data.size(); i++) {
		double temp = data[i].ripple_height;
		if (temp > max) {
			max = temp;
		}
	}
	return max;
}

double Weather::find_min_ripple_height() {
	if (data.size() == 0) {
		return -1;
	}

	double min = data[0].ripple_height;

	for (int i = 0; i < data.size(); i++) {
		double temp = data[i].ripple_height;
		if (temp < min) {
			min = temp;
		}
	}
	return min;
}

// чтение всех данных целиком, потом надо вызывать только при включенном моде Погода
//std::vector<Weather::PointWeatherData> Weather::get_all_weather_data(const std::string& path) {
//	std::string line;
//	std::vector<PointWeatherData> all_data;
//
//	std::ifstream in(path);
//	if (in.is_open())
//	{
//		getline(in, line);
//		const std::regex r(R"(\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\")");
//
//		while (getline(in, line))
//		{
//			std::smatch m;
//			std::regex_search(line, m, r);
//			std::vector<double> temp;
//
//			PointWeatherData data;
//			data.creation_time = m[1].str();
//			data.time = m[2].str();
//			data.latitude = do_work(m[3].str());
//			data.longitude = do_work(m[4].str());
//			data.wind_u = do_work(m[5].str());
//			data.wind_v = do_work(m[6].str());
//			data.wave_height = do_work(m[7].str());
//			data.wave_wind_length = do_work(m[8].str());
//			data.wave_wind_period = do_work(m[9].str());
//			data.wave_direction = do_work(m[10].str());
//			data.ripple_height = do_work(m[11].str());
//			data.ripple_direction = do_work(m[12].str());
//			all_data.push_back(data);
//		}
//	}
//	in.close();
//	return all_data;
//}

std::vector<Weather::PointWeatherData> Weather::get_all_weather_data(const std::string& path) {
	std::string line;
	std::vector<PointWeatherData> all_data;

	std::ifstream in(path);
	if (in.is_open())
	{
		getline(in, line);
		const std::regex r(R"(\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\",\"([^"]*)\")");


		std::string example = "";

		while (getline(in, line))
		{

			std::smatch m;
			std::regex_search(line, m, r);
			std::vector<double> temp;

			PointWeatherData data;
			data.creation_time = m[1].str();
			data.time = m[2].str();

			if (example == "") {
				example = data.time;
			}
			if (data.time != example) {
				break;
			}


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
			all_data.push_back(data);
		}
	}
	in.close();
	return all_data;
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