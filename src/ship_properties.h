#ifndef __SHIP_PROPS__
#define __SHIP_PROPS__

#include <string>

namespace WeatherUtils {
	struct ShipProperties {
	public:
		ShipProperties()
		{
		};
		ShipProperties(int danger_height, int power, int displacement, int length, int delta, int speed, int draft) :
			danger_height(danger_height), power(power), displacement(displacement), length(length), delta(delta), speed(speed), draft(draft) 
		{
		};
		~ShipProperties()
		{
		};

		friend bool operator==(ShipProperties s1, ShipProperties s2) {
			return
				s1.danger_height == s2.danger_height &&
				s1.power == s2.power &&
				s1.displacement == s2.displacement &&
				s1.length == s2.length &&
				s1.delta == s2.delta &&
				s1.speed == s2.speed &&
				s1.draft == s2.draft;
		}

		// danger wave height, centimeters
		int danger_height;
		// "N", kw
		int power;
		// "D", tonn
		int displacement;
		// "L", meters
		int length;
		// real speed calculation constant, 1e-3
		int delta;
		// self ship speed, knot/h
		int speed;
		// ship draft, feet
		int draft;
	};
}

#endif