#pragma once
#include <string>

namespace WeatherUtils {
	struct RefugePlace {
	public:
		RefugePlace();
		RefugePlace(int id, std::string name, double lat, double lon, int ship_class);

		int id;
		std::string name;
		double latitude;
		double longitude;
		int ship_class;
	};
}