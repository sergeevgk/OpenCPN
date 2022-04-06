#pragma once
#include <string>

namespace WeatherUtils {
	struct ShipClass {
	public:
		ShipClass();
		ShipClass(std::string name, int wave_height);

		std::string name;
		int wave_height;
	};
}