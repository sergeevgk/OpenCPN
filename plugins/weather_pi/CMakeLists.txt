# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1.1)
CMAKE_POLICY(SET CMP0043 NEW)

PROJECT(weather_pi)

SET(PACKAGE_NAME weather_pi)
SET(VERBOSE_NAME weather_pi)
SET(TITLE_NAME weather_pi)
SET(CPACK_PACKAGE_CONTACT "nobody")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "1")
SET(VERSION_PATCH "2")

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginPackage.cmake")
