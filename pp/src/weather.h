//////////////////////////////////////////////////////////////////////////////
//
/// weather.h - Weather Class Structures and Functions
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2004-2006 C. W. McHenry
/// Authors: C. W. McHenry (traithe@middle-earth.us)
///          Jonathan W. Webb (sighentist@middle-earth.us)
/// URL: http://www.middle-earth.us
//
/// May includes portions derived from Harshlands
/// Authors: Charles Rand (Rassilon)
/// URL: http://www.harshlands.net
//
/// May include portions derived under license from DikuMUD Gamma (0.0)
/// which are Copyright (C) 1990, 1991 DIKU
/// Authors: Hans Henrik Staerfeldt (bombman@freja.diku.dk)
///          Tom Madson (noop@freja.diku.dk)
///          Katja Nyboe (katz@freja.diku.dk)
///          Michael Seifert (seifert@freja.diku.dk)
///          Sebastian Hammer (quinn@freja.diku.dk)
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _rpie_weather_h
#define _rpie_weather_h

#include <string>

enum fog_type {
	NO_FOG,
	THIN_FOG,
	THICK_FOG
};

enum wind_dir_type {
	WEST_WIND,
	NORTHWEST_WIND,
	NORTH_WIND,
	NORTHEAST_WIND,
	EAST_WIND,
	SOUTHEAST_WIND,
	SOUTH_WIND,
	SOUTHWEST_WIND
};

enum wind_str_type {
	CALM,
	BREEZE,
	WINDY,
	GALE,
	STORMY
};

enum cloud_type {
	CLEAR_SKY,
	LIGHT_CLOUDS,
	HEAVY_CLOUDS,
	OVERCAST
};

enum rain_type {
	NO_RAIN,
	CHANCE_RAIN,
	LIGHT_RAIN,
	STEADY_RAIN,
	HEAVY_RAIN,
	LIGHT_SNOW,
	STEADY_SNOW,
	HEAVY_SNOW
};

enum special_effect {
	NO_EFFECT,
	VOLCANIC_SMOKE,
	FOUL_STENCH,
	LOW_MIST
};

/* Objects loaded by weather code */
#define NUM_WEATHER_OBJECTS 10 
const int weather_objects[NUM_WEATHER_OBJECTS] = { 
	95083, // Fresh snow
	95084, // Old snow
	95085, // Slush (snow + rain)
	95086, // fresh puddles
	95087, // old puddles
	95088, // fresh ash
	95089, // old ash
	95090, // fresh stench
	95091, // old stench
	95092 // Muddy Ash puddles (ash + snow/rain)
};

class Weather
{
private:

public:
	int fog;
	int sunlight;
	int trend;
	int temperature;
	int state;
	int clouds;
	int lightning;
	int wind_dir;
	int wind_speed;
	int special_effect;

	static bool weather_unification (int zone);
};

extern Weather weather_info[];

#endif // _rpie_weather_h
