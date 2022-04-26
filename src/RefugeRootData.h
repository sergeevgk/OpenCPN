#ifndef __ROUTE_REFUGE_INFO__
#define __ROUTE_REFUGE_INFO__

#include <string>
#include <vector>
#include "Route.h"

class Route;

namespace Weather
{
	class RefugeRouteData {
	private:
		// determines the route which is checked for conflicts
		Route route;


	public:
		RefugeRouteData();
		virtual ~RefugeRouteData();
		bool CheckCachedState(Route &route, );

	private:
	};
}

#endif