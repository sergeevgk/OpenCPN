#include "refuge_place.h"

using namespace WeatherUtils;


RefugePlace::RefugePlace()
{
}

RefugePlace::RefugePlace(int id, std::string name, double lat, double lon, std::string ship_class) :
	id(id), name(name), latitude(lat), longitude(lon), ship_class(ship_class)
{
}