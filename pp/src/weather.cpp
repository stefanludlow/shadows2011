/*------------------------------------------------------------------------\
|  weather.c :  Weather Module                        www.middle-earth.us | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"

Weather weather_info[100];

int global_moon_light = 0;
int moon_light[100];
int sun_light = 0;
int desc_weather[100];
AFFECTED_TYPE *world_affects = NULL;

const int sunrise[] = { 7, 6, 6, 6, 6, 5, 4, 4, 5, 5, 6, 6 };
const int sunset[] = { 18, 18, 19, 19, 20, 21, 22, 22, 21, 20, 19, 18 };

const int seasonal_temp[7][12] = {
  {40, 47, 55, 63, 71, 79, 85, 79, 71, 63, 55, 47},	/* Temperate baseline */

  {22, 29, 37, 45, 53, 61, 67, 61, 53, 45, 37, 29},	/* Cool */
  {15, 22, 28, 38, 46, 50, 55, 50, 46, 38, 28, 22},	/* Cold */
  {7, 14, 18, 22, 25, 27, 30, 27, 25, 22, 18, 14},	/* Arctic */

  {55, 57, 60, 65, 73, 81, 89, 81, 73, 65, 60, 53},	/* Hot */
  {60, 62, 65, 70, 78, 86, 94, 86, 78, 70, 65, 58},	/* Hot */
  {75, 77, 80, 85, 93, 101, 109, 101, 93, 85, 80, 73}	/* Desert */
};



void
initialize_weather_zones (void)
{
  // Minas Tirith is chillier due to the mountain range
  zone_table[1].weather_type = WEATHER_COOL;
  zone_table[3].weather_type = WEATHER_COOL;

  // Osgiliath and the Ithilien are generally warm
  zone_table[2].weather_type = WEATHER_TEMPERATE;
  zone_table[4].weather_type = WEATHER_WARM;

  // Minas Morgul is warmer due to Mordor proximity
  zone_table[5].weather_type = WEATHER_HOT;
  zone_table[6].weather_type = WEATHER_HOT;
  
  // New Weather zones for Tur Edendor, at Zapata's request
  zone_table[10].weather_type = WEATHER_TEMPERATE;
  zone_table[66].weather_type = WEATHER_COOL;
  zone_table[67].weather_type = WEATHER_WARM;

  // Pelargir, 100 miles south of Gondor, is hot
  zone_table[13].weather_type = WEATHER_HOT;
  
  // Haradwaith and all that
  zone_table[80].weather_type = WEATHER_DESERT;
  zone_table[81].weather_type = WEATHER_DESERT;
  zone_table[82].weather_type = WEATHER_DESERT;

  zone_table[42].weather_type = WEATHER_COLD;
}

bool Weather::weather_unification (int zone)
{
  bool zone_updated = false;


  // Begin Laecla's Changed To Weather Unification Code
  if ( zone >= 1 && zone <= 99)
    {
      weather_info[zone] = weather_info[40];
      zone_updated = true;
    }
//     End Laecla's Change To Weather Unification 

  // Begin Japheth's "Weather zone unification" changes
/*  if ( zone == 3 || zone == 11 || zone == 8 || zone == 38)
    {
      weather_info[zone] = weather_info[1];
      zone_updated = true;
    }
  else if ( zone == 15)
    {
      weather_info[zone] = weather_info[2];
      zone_updated = true;
    }
  else if ( zone == 43)
    {
      weather_info[zone] = weather_info[42];
      zone_updated = true;
    }
  else if ( zone == 65 || zone == 66)
    {
      weather_info[zone] = weather_info[64];
      zone_updated = true;
    }
  else if (( zone >= 70 && zone <= 79)||(zone == 19)||(zone == 51))
    {
      weather_info[zone] = weather_info[10];
      zone_updated = true;
    }
	else if (zone == 81)
	{
		weather_info[zone] = weather_info[80];
		zone_updated = true;
	}
	else if (zone == 12 || zone == 51 || zone == 54 || zone == 76)
	{
		weather_info[zone] = weather_info[14];
		zone_updated = true;
	}
 else if ( zone == 65 || zone == 66)
    {
      weather_info[zone] = weather_info[64];
      zone_updated = true;
    } 
  // End Japheth's changes 
*/
  return zone_updated;
}

void
weather (int moon_setting, int moon_rise, int moon_set)
{
  int last_temp = 0, last_clouds = 0, last_state = 0;
  int roll = 0, chance_of_rain = 0, last_fog = 0, i = 0;
  char buf[MAX_STRING_LENGTH];
  char storm[MAX_STRING_LENGTH];
  char wind[AVG_STRING_LENGTH] = { '\0' };

  for (i = 0; i <= 99; i++)
    {
      if (Weather::weather_unification (i))
	continue;
      chance_of_rain = 0;

      last_temp = weather_info[i].temperature;
      last_clouds = weather_info[i].clouds;
      last_state = weather_info[i].state;
      last_fog = weather_info[i].fog;

      weather_info[i].temperature =
	seasonal_temp[zone_table[i].weather_type][time_info.month];

      if (time_info.hour == sunrise[time_info.month])
	weather_info[i].trend =
	  (weather_info[i].trend * 2 + number (0, 15)) / 3;

      weather_info[i].temperature += weather_info[i].trend;

  /*** If there is a wind there is a chance it will change direction ***/
  /*
		I do not claim to be knowledgeable in meteorology so this
		code is going to be far from scientific. I am working on
		the following principle, however:

		The wind is more likely to change direction a little than
		to completely turn around and blow in the opposite direction.
		That assumption made, whatever the current direction of the wind,
		the chance of it moving to another direction gets lower as the
		directions get further from the current one, with the least
		likely being the absolute opposite direction.

		- Valarauka
  */
    if ((weather_info[i].wind_speed < STORMY)
	  && (weather_info[i].wind_speed > CALM))
	{
	  if (weather_info[i].wind_dir == WEST_WIND) //currently westerly
	  {
	    /* Check for changes: most likely -> least likely */

		/* Southwest and Northwest are most likely */
		if (!number (0, 5))
		{
			send_outside_zone
		    ("The westerly winds veer a little, coming more now from the southwest.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHWEST_WIND;
		}
		else if (!number (0, 5))
		{
			send_outside_zone
		    ("The westerly winds veer a little, coming more now from the northwest.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHWEST_WIND;
		}

		/* Next most likely are North and South */
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The westerly wind turns and begins to blow more from the north.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTH_WIND;
		}
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The westerly wind turns and begins to blow more from the south.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTH_WIND;
		}

		/* Next most likely are Northeast and Southeast */
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The westerly wind turns dramatically, moving almost back on itself to blow from the southeast.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHEAST_WIND;
		}
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The westerly wind turns dramatically, moving almost back on itself to blow from the northeast.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHEAST_WIND;
		}

		/* Least likely is an Easterly wind*/
		else if (!number (0, 25))
		{
			send_outside_zone
		    ("The westerly wind suddenly reverses, blowing back on itself now from the east.\n\r",
		     i);
		  weather_info[i].wind_dir = EAST_WIND;
		}

		/* If none of these happen, wind remains in the current direction for now */

	  }
	  else if (weather_info[i].wind_dir == SOUTHWEST_WIND) //currently southwesterly
	  {
		/* Check for changes: most likely -> least likely */

		/* South and west are most likely */
		if (!number (0, 5))
		{
			send_outside_zone
		    ("The south westerly winds veer a little, coming more now from the south.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTH_WIND;
		}
		else if (!number (0, 5))
		{
			send_outside_zone
		    ("The south westerly winds veer a little, coming more now from the west.\n\r",
		     i);
		  weather_info[i].wind_dir = WEST_WIND;
		}

		/* Next most likely are Northwest and Southeast */
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The south westerly wind turns and begins to blow more from the north west.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHWEST_WIND;
		}
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The south westerly wind turns and begins to blow more from the south east.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHEAST_WIND;
		}

		/* Next most likely are North and East */
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The south westerly wind turns dramatically, moving almost back on itself to blow from the north.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTH_WIND;
		}
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The south westerly wind turns dramatically, moving almost back on itself to blow from the east.\n\r",
		     i);
		  weather_info[i].wind_dir = EAST_WIND;
		}

		/* Least likely is a northeasterly wind*/
		else if (!number (0, 25))
		{
			send_outside_zone
		    ("The south westerly wind suddenly reverses, blowing back on itself now from the north east.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHEAST_WIND;
		}

		/* If none of these happen, wind remains in the current direction for now */
	  }
	  else if (weather_info[i].wind_dir == SOUTH_WIND) //currently southerly
	  {
		/* Check for changes: most likely -> least likely */

		/* Southeast and Southwest are most likely */
		if (!number (0, 5))
		{
			send_outside_zone
		    ("The southerly winds veer a little, coming more now from the south east.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHEAST_WIND;
		}
		else if (!number (0, 5))
		{
			send_outside_zone
		    ("The southerly winds veer a little, coming more now from the south west.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHWEST_WIND;
		}

		/* Next most likely are west and east */
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The southerly wind turns and begins to blow more from the west.\n\r",
		     i);
		  weather_info[i].wind_dir = WEST_WIND;
		}
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The southerly wind turns and begins to blow more from the east.\n\r",
		     i);
		  weather_info[i].wind_dir = EAST_WIND;
		}

		/* Next most likely are Northwest and Northeast */
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The southerly wind turns dramatically, moving almost back on itself to blow from the north west.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHWEST_WIND;
		}
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The southerly wind turns dramatically, moving almost back on itself to blow from the north east.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHEAST_WIND;
		}

		/* Least likely is a northerly wind*/
		else if (!number (0, 25))
		{
			send_outside_zone
		    ("The southerly wind suddenly reverses, blowing back on itself now from the north.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTH_WIND;
		}

		/* If none of these happen, wind remains in the current direction for now */
	  }
	  else if (weather_info[i].wind_dir == SOUTHEAST_WIND) //currently southeasterly
	  {
		/* Check for changes: most likely -> least likely */

		/* East and South are most likely */
		if (!number (0, 5))
		{
			send_outside_zone
		    ("The south easterly winds veer a little, coming more now from the east.\n\r",
		     i);
		  weather_info[i].wind_dir = EAST_WIND;
		}
		else if (!number (0, 5))
		{
			send_outside_zone
		    ("The south easterly winds veer a little, coming more now from the south.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTH_WIND;
		}

		/* Next most likely are southwest and northeast */
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The south easterly wind turns and begins to blow more from the south west.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHWEST_WIND;
		}
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The south easterly wind turns and begins to blow more from the north east.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHEAST_WIND;
		}

		/* Next most likely are West and North */
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The south easterly wind turns dramatically, moving almost back on itself to blow from the west.\n\r",
		     i);
		  weather_info[i].wind_dir = WEST_WIND;
		}
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The south easterly wind turns dramatically, moving almost back on itself to blow from the north.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTH_WIND;
		}

		/* Least likely is a northwesterly wind*/
		else if (!number (0, 25))
		{
			send_outside_zone
		    ("The westerly wind suddenly reverses, blowing back on itself now from the north west.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHWEST_WIND;
		}

		/* If none of these happen, wind remains in the current direction for now */
	  }
	  else if (weather_info[i].wind_dir == EAST_WIND) //currently easterly
	  {
		/* Check for changes: most likely -> least likely */

		/* Northeast and Southeast are most likely */
		if (!number (0, 5))
		{
			send_outside_zone
		    ("The easterly winds veer a little, coming more now from the north east.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHEAST_WIND;
		}
		else if (!number (0, 5))
		{
			send_outside_zone
		    ("The easterly winds veer a little, coming more now from the south east.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHEAST_WIND;
		}

		/* Next most likely are north and south */
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The easterly wind turns and begins to blow more from the north.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTH_WIND;
		}
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The easterly wind turns and begins to blow more from the south.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTH_WIND;
		}

		/* Next most likely are Northwest and Southwest */
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The easterly wind turns dramatically, moving almost back on itself to blow from the north west.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHWEST_WIND;
		}
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The easterly wind turns dramatically, moving almost back on itself to blow from the south west.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHWEST_WIND;
		}

		/* Least likely is a westerly wind*/
		else if (!number (0, 25))
		{
			send_outside_zone
		    ("The easterly wind suddenly reverses, blowing back on itself now from the west.\n\r",
		     i);
		  weather_info[i].wind_dir = WEST_WIND;
		}

		/* If none of these happen, wind remains in the current direction for now */
	  }
	  else if (weather_info[i].wind_dir == NORTHEAST_WIND) //currently northeasterly
	  {
		/* Check for changes: most likely -> least likely */

		/* North and East are most likely */
		if (!number (0, 5))
		{
			send_outside_zone
		    ("The north easterly winds veer a little, coming more now from the north.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTH_WIND;
		}
		else if (!number (0, 5))
		{
			send_outside_zone
		    ("The north easterly winds veer a little, coming more now from the east.\n\r",
		     i);
		  weather_info[i].wind_dir = EAST_WIND;
		}

		/* Next most likely are northwest and southeast */
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The north easterly wind turns and begins to blow more from the north west.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHWEST_WIND;
		}
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The north easterly wind turns and begins to blow more from the south east.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHEAST_WIND;
		}

		/* Next most likely are West and South */
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The north easterly wind turns dramatically, moving almost back on itself to blow from the west.\n\r",
		     i);
		  weather_info[i].wind_dir = WEST_WIND;
		}
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The north easterly wind turns dramatically, moving almost back on itself to blow from the south.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTH_WIND;
		}

		/* Least likely is a southwesterly wind*/
		else if (!number (0, 25))
		{
			send_outside_zone
		    ("The north easterly wind suddenly reverses, blowing back on itself now from the south west.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHWEST_WIND;
		}

		/* If none of these happen, wind remains in the current direction for now */
	  }
	  else if (weather_info[i].wind_dir == NORTH_WIND) //currently northerly
	  {
		/* Check for changes: most likely -> least likely */

		/* Northeast and Northwest are most likely */
		if (!number (0, 5))
		{
			send_outside_zone
		    ("The northerly winds veer a little, coming more now from the north east.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHEAST_WIND;
		}
		else if (!number (0, 5))
		{
			send_outside_zone
		    ("The northerly winds veer a little, coming more now from the north north west.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHWEST_WIND;
		}

		/* Next most likely are west and east */
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The northerly wind turns and begins to blow more from the west.\n\r",
		     i);
		  weather_info[i].wind_dir = WEST_WIND;
		}
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The northerly wind turns and begins to blow more from the east.\n\r",
		     i);
		  weather_info[i].wind_dir = EAST_WIND;
		}

		/* Next most likely are Southeast and Southwest */
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The northerly wind turns dramatically, moving almost back on itself to blow from the south east.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHEAST_WIND;
		}
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The northerly wind turns dramatically, moving almost back on itself to blow from the south west.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHWEST_WIND;
		}

		/* Least likely is a southerly wind*/
		else if (!number (0, 25))
		{
			send_outside_zone
		    ("The northerly wind suddenly reverses, blowing back on itself now from the south.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTH_WIND;
		}

		/* If none of these happen, wind remains in the current direction for now */
	  }
	  else if (weather_info[i].wind_dir == NORTHWEST_WIND) //currently northwesterly
	  {
		/* Check for changes: most likely -> least likely */

		/* North and West are most likely */
		if (!number (0, 5))
		{
			send_outside_zone
		    ("The north westerly winds veer a little, coming more now from the north.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTH_WIND;
		}
		else if (!number (0, 5))
		{
			send_outside_zone
		    ("The north westerly winds veer a little, coming more now from the west.\n\r",
		     i);
		  weather_info[i].wind_dir = WEST_WIND;
		}

		/* Next most likely are southwest and northeast */
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The north westerly wind turns and begins to blow more from the southwest.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHWEST_WIND;
		}
		else if (!number (0, 10))
		{
			send_outside_zone
		    ("The north westerly wind turns and begins to blow more from the northeast.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTHEAST_WIND;
		}

		/* Next most likely are east and South */
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The north westerly wind turns dramatically, moving almost back on itself to blow from the east.\n\r",
		     i);
		  weather_info[i].wind_dir = EAST_WIND;
		}
		else if (!number (0, 20))
		{
			send_outside_zone
		    ("The north westerly wind turns dramatically, moving almost back on itself to blow from the south.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTH_WIND;
		}

		/* Least likely is a southeasterly wind*/
		else if (!number (0, 25))
		{
			send_outside_zone
		    ("The north westerly wind suddenly reverses, blowing back on itself now from the southeast.\n\r",
		     i);
		  weather_info[i].wind_dir = SOUTHEAST_WIND;
		}

		/* If none of these happen, wind remains in the current direction for now */
	  }
	}
	/*** End wind direction code ***/

      if (!number (0, 15) && weather_info[i].wind_speed)
	{
	  roll = number (0, 99);
	  roll += weather_info[i].temperature / 3;

	  if ((time_info.season == SPRING) || (time_info.season == AUTUMN))
	    {
	      if (roll < 20)
		{
		  if (last_clouds == OVERCAST)
		    weather_info[i].clouds = HEAVY_CLOUDS;
		  else
		    weather_info[i].clouds = CLEAR_SKY;
		}
	      else if (roll < 45)
		weather_info[i].clouds = LIGHT_CLOUDS;
	      else if (roll < 80)
		weather_info[i].clouds = HEAVY_CLOUDS;
	      else
		{
		  if (last_clouds == CLEAR_SKY)
		    weather_info[i].clouds = LIGHT_CLOUDS;
		  else
		    weather_info[i].clouds = OVERCAST;
		}
	    }
	  else if (time_info.season == SUMMER)
	    {
	      if (roll < 50)
		{
		  if (last_clouds == OVERCAST)
		    weather_info[i].clouds = HEAVY_CLOUDS;
		  else
		    weather_info[i].clouds = CLEAR_SKY;
		}
	      else if (roll < 80)
		weather_info[i].clouds = LIGHT_CLOUDS;
	      else if (roll < 90)
		weather_info[i].clouds = HEAVY_CLOUDS;
	      else
		{
		  if (last_clouds == CLEAR_SKY)
		    weather_info[i].clouds = LIGHT_CLOUDS;
		  else
		    weather_info[i].clouds = OVERCAST;
		}
	    }
	  else
	    {
	      if (roll < 10)
		{
		  if (last_clouds == OVERCAST)
		    weather_info[i].clouds = HEAVY_CLOUDS;
		  else
		    weather_info[i].clouds = CLEAR_SKY;
		}
	      else if (roll < 25)
		weather_info[i].clouds = LIGHT_CLOUDS;
	      else if (roll < 75)
		weather_info[i].clouds = HEAVY_CLOUDS;
	      else
		{
		  if (last_clouds == CLEAR_SKY)
		    weather_info[i].clouds = LIGHT_CLOUDS;
		  else
		    weather_info[i].clouds = OVERCAST;
		}
	    }
	}

	  if (weather_info[i].wind_dir == NORTH_WIND)
	  {
		  sprintf (wind, "northerly");
	  }
	  else if(weather_info[i].wind_dir == NORTHEAST_WIND)
	  {
		  sprintf (wind, "north easterly");
	  }
	  else if(weather_info[i].wind_dir == EAST_WIND)
	  {
		  sprintf (wind, "easterly");
	  }
	  else if(weather_info[i].wind_dir == SOUTHEAST_WIND)
	  {
		  sprintf (wind, "south easterly");
	  }
	  else if(weather_info[i].wind_dir == SOUTH_WIND)
	  {
		  sprintf (wind, "southerly");
	  }
	  else if(weather_info[i].wind_dir == SOUTHWEST_WIND)
	  {
		  sprintf (wind, "south westerly");
	  }
	  else if(weather_info[i].wind_dir == NORTHWEST_WIND)
	  {
		  sprintf (wind, "north westerly");
	  }
	  else
	  {
		  sprintf (wind, "westerly");
	  }

      if (weather_info[i].fog < THICK_FOG)
	{
	  if ((weather_info[i].clouds == CLEAR_SKY)
	      && (weather_info[i].clouds != last_clouds))
	    {
			sprintf (buf,
			       "The clouds are born away upon the prevailing %s winds and clear skies open up above.\n\r",
			       wind);
		    send_outside_zone (buf, i);

	    }

	  if ((weather_info[i].clouds == LIGHT_CLOUDS)
	      && (weather_info[i].clouds != last_clouds))
	    {
	      
		  if (last_clouds > weather_info[i].clouds)
		  {
			  sprintf (buf,
			       "The cloud cover begins to clear, carried away upon the prevailing %s winds.\n\r",
			       wind);
			  send_outside_zone (buf, i);
		  }
		  if (last_clouds < weather_info[i].clouds)
		  {
			  sprintf (buf,
			       "Wisplike clouds drift in overhead, carried upon the prevailing %s winds.\n\r",
			       wind);
			  send_outside_zone (buf, i);
		  }  
	    }

	  if ((weather_info[i].clouds == HEAVY_CLOUDS)
	      && (weather_info[i].clouds != last_clouds))
	    {
		  if (last_clouds < weather_info[i].clouds)
		  {
			  sprintf (buf,
			       "A host of clouds marches overhead upon the prevailing %s winds.\n\r",
			       wind);
			  send_outside_zone (buf, i);
		  }
		  if (last_clouds > weather_info[i].clouds)
		  {
			  sprintf (buf,
			       "Small patches of sky open up as the storm clouds drift away on the prevailing %s winds.\n\r",
			       wind);
			  send_outside_zone (buf, i);
		  }
	    }

	  if ((weather_info[i].clouds == OVERCAST)
	      && (weather_info[i].clouds != last_clouds))
	    {
		  if (sun_light == 1)
		    send_outside_zone
		      ("The prevailing winds bring a blanket of thick storm clouds to obscure Anor.\n\r",
		       i);
		  else
		    send_outside_zone
		      ("The prevailing winds bring a blanket of thick storm clouds into the sky.\n\r",
		       i);
	    }
	}

      if (weather_info[i].clouds != last_clouds)
	{			/*   Is the new front a rain front?   */

	  if (time_info.season == SPRING)	/*   Spring rains   */
	    chance_of_rain = 20;

	  if (weather_info[i].clouds == CLEAR_SKY)	/*   More clouds = Higher chance of rain   */
	    chance_of_rain = 0;
	  else
	    chance_of_rain += (weather_info[i].clouds * 15);

	  if (zone_table[i].weather_type == WEATHER_DESERT)
		chance_of_rain = 0;
	
	  if (number (0, 99) < chance_of_rain)
	    {
	      weather_info[i].state = CHANCE_RAIN;
	    }
	  else if ((last_state > CHANCE_RAIN) && (last_state < LIGHT_SNOW))
	    {
	      weather_info[i].state = NO_RAIN;
	      send_outside_zone ("The rain passes.\n\r", i);
	    }
	  else if (last_state > HEAVY_RAIN)
	    {
	      weather_info[i].state = NO_RAIN;
	      send_outside ("It stops snowing.\n\r");
	    }
	}

      /*   Lightning is more common the closer you get to midsummer. I wanted it to be more common   */
      /*   with higher temperatures, but we haven't determined temp and we need to know now.   */

      if ((weather_info[i].clouds > LIGHT_CLOUDS)
	  && (weather_info[i].state > NO_RAIN))
	{
	  if (number (35, 350) <
	      seasonal_temp[zone_table[i].weather_type][time_info.month])
	    {
	      if (number (1, 10) && number (1, 3) < weather_info[i].clouds)
		{
		  weather_info[i].lightning = 1;
		  send_outside_zone
		    ("Lightning flashes across the heavens.\n\r", i);
		}
	      else
		weather_info[i].lightning = 0;
	    }
	}

      if (!number (0, 4))
	{
	  roll = number (-1, 1);
	  switch (roll)
	    {
	    case -1:
	      if (weather_info[i].wind_speed == CALM)
		break;
	      if (weather_info[i].wind_speed == BREEZE && number (0, 1))
		break;
	      weather_info[i].wind_speed -= 1;
	      if (weather_info[i].wind_speed == CALM)
		send_outside_zone ("The winds die and the air stills.\n\r",
				   i);
	      if (weather_info[i].wind_speed == BREEZE)
		send_outside_zone ("The wind dies down to a mild breeze.\n\r",
				   i);
	      if (weather_info[i].wind_speed == WINDY)
		send_outside_zone
		  ("The gale winds die down to a steady current.\n\r", i);
	      if (weather_info[i].wind_speed == GALE)
		send_outside_zone
		  ("The stormy winds slow to a steady gale.\n\r", i);
	      break;

	    case 1:
	      sprintf (storm, "wind storm");
	      if (weather_info[i].state > CHANCE_RAIN)
		sprintf (storm, "rain storm");
	      if (weather_info[i].lightning)
		sprintf (storm, "thunder storm");
	      if (weather_info[i].state > HEAVY_RAIN)
		sprintf (storm, "blizzard");
	      if (weather_info[i].wind_speed == STORMY)
		{
		  send_outside_zone
		    ("The storm winds slow, leaving a steady gale in their wake.\n\r",
		     i);
		  weather_info[i].wind_speed -= 1;
		  break;
		}
	      if (weather_info[i].wind_speed == CALM)
		send_outside_zone ("A capricious breeze picks up.\n\r", i);
	      if (weather_info[i].wind_speed == BREEZE)
		{
		  if (number (0, 1))
		    break;
		  send_outside_zone
		    ("The breeze strengthens into a steady wind.\n\r", i);
		}
	      if (weather_info[i].wind_speed == WINDY)
		{
		  if (!number (0, 3))
		    break;
		  if (weather_info[i].state < LIGHT_RAIN)
		    send_outside_zone
		      ("The winds grow fierce, building into a strong gale.\n\r",
		       i);
		  else
		    {
		      if (weather_info[i].state > HEAVY_RAIN)
			sprintf (storm, "snow storm");
		      sprintf (buf,
			       "The winds grow fierce, building into a mild %s.\n\r",
			       storm);
		      send_outside_zone (buf, i);
		    }
		}
	      if (weather_info[i].wind_speed == GALE)
		{
		  if (!number (0, 5))
		    break;
		  sprintf (buf,
			   "The winds begin to rage, and a fierce %s is born.\n\r",
			   storm);
		  send_outside_zone (buf, i);
		}
	      weather_info[i].wind_speed += 1;
	      break;
	    }
	}

      /*   Wind Chill - This is FAR from scientific, but I didnt want winds to totally take over temperatures. - Koldryn  */
      if (weather_info[i].wind_dir == NORTH_WIND)
	{
	  weather_info[i].temperature -= weather_info[i].wind_speed * 2;
	  roll = 0 - weather_info[i].wind_speed * 2;
	}

      if (weather_info[i].wind_dir == WEST_WIND)
	{
	  weather_info[i].temperature += (5 - weather_info[i].wind_speed * 2);
	  roll = 0 + (5 - weather_info[i].wind_speed * 2);
	}

      /*   Angle of Sunlight   */
      if (sun_light)
	{
	  roll = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);

	  if (time_info.hour > roll)
	    roll =
	      (sunset[time_info.month] -
	       time_info.hour) * 100 / (sunset[time_info.month] -
					roll) * 15 / 100;
	  else if (time_info.hour == roll)
	    roll = 15;
	  else if (time_info.hour < roll)
	    roll =
	      (time_info.hour - sunrise[time_info.month]) * 100 / (roll -
								   sunrise
								   [time_info.
								    month]) *
	      15 / 100;

	  weather_info[i].temperature += roll;
	}

      /*   Cloud Chill, which applies only in the daytime - This is not scientific.   */
      if (sun_light)
	{
	  if (weather_info[i].clouds == LIGHT_CLOUDS)
	    weather_info[i].temperature -= ((roll * 3) / 10);
	  if (weather_info[i].clouds == HEAVY_CLOUDS)
	    weather_info[i].temperature -= ((roll * 6) / 10);
	  if (weather_info[i].clouds == OVERCAST)
	    weather_info[i].temperature -= ((roll * 9) / 10);
	  weather_info[i].temperature = ((weather_info[i].temperature + last_temp * 2) / 3);	/*   Limits Drastic Immediate Changes   */
	}
      else
	{
	  if (zone_table[i].weather_type == WEATHER_DESERT)
	    weather_info[i].temperature -= 50;
	  else if ((time_info.season == SPRING) || (time_info.season == AUTUMN))	/*   Gradual Nighttime Cooling   */
	    weather_info[i].temperature -= 10;
	  else if (time_info.season == SUMMER)
	    weather_info[i].temperature -= 15;
	  else
	    weather_info[i].temperature -= 5;
	  roll = 0;
	  if (time_info.hour != sunset[time_info.month])
	    roll = 5;
	  weather_info[i].temperature =
	    ((weather_info[i].temperature + (last_temp + roll) * 4) / 5);
	  weather_info[i].temperature -= 5;	/*   Immediate Nighttime Chill   */
	}

      /*   Will it rain?   */
      if (weather_info[i].state == CHANCE_RAIN)
	{
	  chance_of_rain = (weather_info[i].clouds * 15);
	  if (time_info.season == SUMMER)
	    chance_of_rain -= 20;
	  if (time_info.season == AUTUMN)
	    chance_of_rain -= 10;
	  if (time_info.season == SPRING)
	    chance_of_rain += 10;
	  chance_of_rain = MAX (1, chance_of_rain);
	  if (weather_info[i].lightning && number (0, 39))	/*   Its very rare for lightning not to cause rain   */
	    chance_of_rain = 100;

	  if (number (0, 99) < chance_of_rain)
	    weather_info[i].state = weather_info[i].clouds + 1;
	}

      /*   If its going to rain, how hard?   */
      if (weather_info[i].state > CHANCE_RAIN)
	{
	  if (weather_info[i].state > HEAVY_RAIN)
	    weather_info[i].state -= 3;
	  roll = number (0, 99);
	  if (weather_info[i].clouds == LIGHT_CLOUDS)
	    {
	      if (roll < 40)
		weather_info[i].state = CHANCE_RAIN;
	      else if (roll < 85)
		weather_info[i].state = LIGHT_RAIN;
	      else
		weather_info[i].state = STEADY_RAIN;
	    }
	  if (weather_info[i].clouds == HEAVY_CLOUDS)
	    {
	      if (roll < 30)
		weather_info[i].state = CHANCE_RAIN;
	      else if (roll < 60)
		weather_info[i].state = LIGHT_RAIN;
	      else if (roll < 90)
		weather_info[i].state = STEADY_RAIN;
	      else
		weather_info[i].state = HEAVY_RAIN;
	    }
	  if (weather_info[i].clouds == OVERCAST)
	    {
	      if (roll < 20)
		weather_info[i].state = CHANCE_RAIN;
	      else if (roll < 50)
		weather_info[i].state = LIGHT_RAIN;
	      else if (roll < 80)
		weather_info[i].state = STEADY_RAIN;
	      else
		weather_info[i].state = HEAVY_RAIN;
	    }

	  /*   Is it rain or snow?   */
	  if ((weather_info[i].temperature < 32) 
	      && (weather_info[i].state > CHANCE_RAIN))	
	    {
	      if ((weather_info[i].state += 3) == HEAVY_SNOW
		  && (weather_info[i].temperature < 20))
		weather_info[i].state--;
	    }

	  /*   Lightning should never allow existing rain to stop   */
	  if (weather_info[i].lightning 
	      && (weather_info[i].state == CHANCE_RAIN))
	    weather_info[i].state = weather_info[i].clouds + 1;
	}

      /*   If the rain has changed, display a message.   */
      if ((weather_info[i].state != last_state)
	  && (weather_info[i].state != NO_RAIN))
	{
	  if ((weather_info[i].state == CHANCE_RAIN)
	      && (last_state > CHANCE_RAIN) && (last_state < LIGHT_SNOW))
	    send_outside_zone ("The rain fades and stops.\n\r", i);
	  if ((weather_info[i].state == CHANCE_RAIN)
	      && (last_state > HEAVY_RAIN))
	    send_outside_zone ("It stops snowing.\n\r", i);
	  if (weather_info[i].state == LIGHT_RAIN)
	    send_outside_zone
	      ("A light sprinkling of rain falls from the sky.\n\r", i);
	  if (weather_info[i].state == STEADY_RAIN)
	    send_outside_zone ("A steady rain falls from the sky.\n\r", i);
	  if (weather_info[i].state == HEAVY_RAIN)
	    send_outside_zone ("Pouring rain showers down upon the land.\n\r",
			       i);
	  if (weather_info[i].state == LIGHT_SNOW)
	    send_outside_zone ("A light snow falls lazily from the sky.\n\r",
			       i);
	  if (weather_info[i].state == STEADY_SNOW)
	    send_outside_zone ("Snow falls steadily from the sky.\n\r", i);
	  if (weather_info[i].state == HEAVY_SNOW)
	    send_outside_zone ("Blinding snows fall from the sky.\n\r", i);
	}

      if (weather_info[i].fog)
	{
	  if (weather_info[i].wind_speed == WINDY)
	    weather_info[i].fog -= 1;
	  if (weather_info[i].wind_speed > WINDY)
	    {
	      weather_info[i].fog = NO_FOG;
	      send_outside_zone
		("The fog churns in the wind and is swept away.\n\r", i);
	    }
	  if (time_info.hour > sunrise[time_info.month]
	      && weather_info[i].clouds == CLEAR_SKY)
	    weather_info[i].fog -= 1;
	  if (time_info.hour == 9)
	    weather_info[i].fog -= 1;
	  if (time_info.hour == 12)
	    weather_info[i].fog -= 1;
	  if (time_info.hour > sunrise[time_info.month]
	      && weather_info[i].clouds > CLEAR_SKY)
	    {
	      roll = number (1, 4);
	      if (roll > weather_info[i].clouds)
		weather_info[i].fog -= 1;
	    }
	  if (weather_info[i].fog < NO_FOG)
	    weather_info[i].fog = NO_FOG;
	  if (weather_info[i].fog == THIN_FOG && last_fog == THICK_FOG)
	    send_outside_zone ("The fog thins a little.\n\r", i);
	  if (weather_info[i].fog == NO_FOG)
	    send_outside_zone ("The fog lifts.\n\r", i);
	}

      /*  If its after midnight, before dawn, within 3 hours of dawn, there is no fog, and there is no artificial sunlight....   */
      if ((sunrise[time_info.month] < (time_info.hour + 4))
	  && (sun_light == 0) && (time_info.hour < sunrise[time_info.month])
	  && (weather_info[i].fog == NO_FOG))
	{
	  switch (time_info.season)
	    {
	    case WINTER:
	      chance_of_rain = 25;
	      break;		/*   Chance of Fog   */
	    case AUTUMN:
	      chance_of_rain = 15;
	      break;
	    case SPRING:
	      chance_of_rain = 8;
	      break;
	    case SUMMER:
	      chance_of_rain = 5;
	      break;
	    }
	  roll = number (1, 100);
	  if (weather_info[i].wind_speed == BREEZE && roll < chance_of_rain)
	    {
	      weather_info[i].fog = THIN_FOG;
	      send_outside_zone ("A thin fog wafts in on the breeze.\n\r", i);
	    }
	  if (weather_info[i].wind_speed == CALM && roll < chance_of_rain)
	    {
	      weather_info[i].fog = THICK_FOG;
	      send_outside_zone
		("A thick fog begins to condense in the still air.\n\r", i);
	    }
	}

      /*
         sprintf (buf,"%d:00 %d degrees F\n\r",time_info.hour,weather_info.temperature);
         send_to_gods (buf);
       */

      desc_weather[i] = WR_NORMAL;

      if (weather_info[i].clouds > CLEAR_SKY)
	desc_weather[i] = WR_CLOUDY;

      if (weather_info[i].fog)
	desc_weather[i] = WR_FOGGY;

      if (weather_info[i].state > CHANCE_RAIN)
	{
	  desc_weather[i] = WR_RAINY;
	  if (weather_info[i].wind_speed == STORMY)
	    desc_weather[i] = WR_STORMY;
	}

      if (weather_info[i].state > HEAVY_RAIN)
	{
	  desc_weather[i] = WR_SNOWY;
	  if (weather_info[i].wind_speed == STORMY)
	    desc_weather[i] = WR_BLIZARD;
	}

      if (time_info.hour < sunrise[time_info.month]
	  && time_info.hour > sunset[time_info.month])
	{
	  switch (desc_weather[i])
	    {
	    case WR_NORMAL:
	      desc_weather[i] = WR_NIGHT;
	      break;
	    case WR_FOGGY:
	      desc_weather[i] = WR_NIGHT_FOGGY;
	      break;
	    case WR_CLOUDY:
	      desc_weather[i] = WR_NIGHT;
	      break;
	    case WR_RAINY:
	      desc_weather[i] = WR_NIGHT_RAINY;
	      break;
	    case WR_SNOWY:
	      desc_weather[i] = WR_NIGHT_SNOWY;
	      break;
	    case WR_BLIZARD:
	      desc_weather[i] = WR_NIGHT_BLIZARD;
	      break;
	    case WR_STORMY:
	      desc_weather[i] = WR_NIGHT_STORMY;
	      break;
	    }
	}

      if (weather_info[i].clouds < HEAVY_CLOUDS
	  && weather_info[i].fog < THICK_FOG
	  && weather_info[i].state < HEAVY_SNOW)
	{
	  if (moon_setting)
	    send_outside_zone ("Ithil hangs low in the sky.\n\r", i);
	  if (moon_set)
	    send_outside_zone
	      ("Ithil slowly sinks from the sky, guided by Tilion to its rest.\n\r",
	       i);
	  if (moon_rise)
	    send_outside_zone
	      ("Ithil rises with stately grace into the sky.\n\r", i);
	  moon_light[i] = global_moon_light;
	}
      else
	{
	  moon_light[i] = 0;
	}
    }
}

int
is_leap_year (int year)
{
  if (year % 4 == 0)
    {
      if (year % 100 == 0)
	{
	  if (year % 400 == 0)
	    return 1;
	  else
	    return 0;
	}
      return 1;
    }

  return 0;
}

void
weather_and_time (int mode)
{
  int moon_rising = 0;
  int moon_setting = 0;
  int moon_set = 0;
  int moon_q;
  int d_day;
  int i;
  bool new_day = false;

  next_hour_update += 900;	/* This is a mud hour; 60*60/4 */

  sun_light = 0;
  /*global_moon_light = 0; */

  time_info.hour++;

  if (time_info.hour >= 24)
    {
      time_info.day++;
      time_info.hour = 0;
      new_day = true;
    }

  if (time_info.day >= 30 && new_day)
    {
      if (!time_info.holiday)
	time_info.month++;
      if (time_info.month >= 12)
	{
	  time_info.month = 0;
	}
      if (time_info.month == 0 || time_info.month == 1
	  || time_info.month == 11)
	time_info.season = WINTER;
      else if (time_info.month >= 2 && time_info.month <= 4)
	time_info.season = SPRING;
      else if (time_info.month > 4 && time_info.month <= 7)
	time_info.season = SUMMER;
      else if (time_info.month > 7 && time_info.month <= 10)
	time_info.season = AUTUMN;

      if (time_info.holiday == HOLIDAY_METTARE)
	{
	  time_info.holiday = HOLIDAY_YESTARE;
	  time_info.year++;
	}
      else if (time_info.holiday == HOLIDAY_LOENDE
	       && is_leap_year (time_info.year))
	time_info.holiday = HOLIDAY_ENDERI;
      else if (time_info.holiday == HOLIDAY_ENDERI)
	{
	  time_info.holiday = 0;
	  time_info.day = 0;
	}
      else if (time_info.month == 0)
	{
	  if (time_info.holiday != HOLIDAY_YESTARE)
	    time_info.holiday = HOLIDAY_METTARE;
	  else
	    {
	      time_info.holiday = 0;
	      time_info.day = 0;
	    }
	}
      else if (time_info.month == 3)
	{
	  if (time_info.holiday != HOLIDAY_TUILERE)
	    time_info.holiday = HOLIDAY_TUILERE;
	  else
	    {
	      time_info.holiday = 0;
	      time_info.day = 0;
	    }
	}
      else if (time_info.month == 6)
	{
	  if (time_info.holiday != HOLIDAY_LOENDE)
	    time_info.holiday = HOLIDAY_LOENDE;
	  else
	    {
	      time_info.holiday = 0;
	      time_info.day = 0;
	    }
	}
      else if (time_info.month == 9)
	{
	  if (time_info.holiday != HOLIDAY_YAVIERE)
	    time_info.holiday = HOLIDAY_YAVIERE;
	  else
	    {
	      time_info.holiday = 0;
	      time_info.day = 0;
	    }
	}
      else
	time_info.day = 0;
    }

  if (time_info.month >= 12)
    {
      time_info.year++;
      time_info.month = 0;
    }

  if (sunrise[time_info.month] <= time_info.hour &&
      sunset[time_info.month] > time_info.hour)
    sun_light = 1;

  if (sunrise[time_info.month] == time_info.hour + 1)
    send_outside ("A glow illuminates the eastern horizon.\n\r");

  if (sunrise[time_info.month] == time_info.hour)
    send_outside
      ("Anor's fiery exterior slowly lifts itself up over the eastern horizon beneath Arien's unwavering guidance.\n\r");

  if (sunset[time_info.month] == time_info.hour + 1)
    send_outside
      ("Anor begins dipping below the western horizon, guided to its respite by Arien.\n\r");

  if (sunset[time_info.month] == time_info.hour)
    send_outside
      ("Anor sets in a fiery cascade of brilliant color upon the western horizon.\n\r");

  d_day = (time_info.day + 15) % 30;
  moon_q = d_day * 24 / 30;

  for (i = -7; i <= 6; i++)
    {
      if (moon_q == (24 + time_info.hour + i) % 24)
	{
	  if (i == -7)
	    {
	      moon_set = 1;
	      global_moon_light = 0;
	    }
	  /*else
	     global_moon_light = 1; */

	  if (i == -6)
	    {
	      moon_setting = 1;
	    }
	  if (i == 6)
	    {
	      moon_rising = 1;
	      global_moon_light = (time_info.hour > 16
				   && time_info.hour < 20) ? 1 : 0;
	    }
	}
    }

  if (is_room_affected (world_affects, MAGIC_WORLD_SOLAR_FLARE))
    sun_light = 1;

  if (is_room_affected (world_affects, MAGIC_WORLD_CLOUDS))
    sun_light = 0;

  if (is_room_affected (world_affects, MAGIC_WORLD_MOON))
    global_moon_light = 1;

  weather (moon_setting, moon_rising, moon_set);
}

int
weather_object_exists(OBJ_DATA * list, int vnum)
{
  for (; list; list = list->next_content)
    if (list->nVirtual == vnum)
      return 1;

  return 0;
}


/*
load_weather_obj()

This function is called every time do_look is called for a room
and checks the weather in room being looked at to see if the appropriate
objects for the current weather are present and if not loads them as required.

The objects

*/
void
load_weather_obj(ROOM_DATA *troom)
{
	OBJ_DATA *obj = NULL;
	int number = 0;

	/* If it is raining */
	if((weather_info[troom->zone].state > CHANCE_RAIN)
		&&(weather_info[troom->zone].state < LIGHT_SNOW))
	{
		//if there is snow in the room replace it with slush
		if((weather_object_exists(troom->contents, weather_objects[0]))||
			(weather_object_exists(troom->contents, weather_objects[1])))
		{
			//remove snow from room
			if((obj = get_obj_in_list_num (weather_objects[0], troom->contents)))
				extract_obj(obj);
			if((obj = get_obj_in_list_num (weather_objects[1], troom->contents)))
				extract_obj(obj);

			//load slush in room
			obj = load_object(weather_objects[2]);
			obj_to_room (obj, troom->nVirtual);
		}
		else
		{
			//if there is ash, replace it with muddy ash
			if((weather_object_exists(troom->contents, weather_objects[5]))||
				(weather_object_exists(troom->contents, weather_objects[6])))
			{
				//remove ash
				if((obj = get_obj_in_list_num (weather_objects[5], troom->contents)))
					extract_obj(obj);
				if((obj = get_obj_in_list_num (weather_objects[6], troom->contents)))
					extract_obj(obj);

				//if not already muddy ash in the room load some
				if(!(obj = get_obj_in_list_num (weather_objects[0], troom->contents)))
				{
					obj = load_object(weather_objects[9]);
					obj_to_room (obj, troom->nVirtual);
				}
			}
			else
			{
				//if there are old puddles replace them with new ones
				if(weather_object_exists(troom->contents, weather_objects[4]))
				{
					//remove old puddles from room
					if((obj = get_obj_in_list_num (weather_objects[4], troom->contents)))
						extract_obj(obj);

					//load fresh puddles in room
					obj = load_object(weather_objects[3]);
					obj_to_room (obj, troom->nVirtual);
				}
				else
				{
					//if there are no puddles or slush or muddy ash load puddles
					if((!weather_object_exists(troom->contents, weather_objects[3]))&&
						(!weather_object_exists(troom->contents, weather_objects[2]))&&
						(!weather_object_exists(troom->contents, weather_objects[9])))
					{
						//load fresh puddles in room
						obj = load_object(weather_objects[3]);
						obj_to_room (obj, troom->nVirtual);
					}
				}
			}
		}
	}
	
	/* If it is snowing */
	else if(weather_info[troom->zone].state > HEAVY_RAIN)
	{
		//if there is rain in the room replace it with slush
		if((weather_object_exists(troom->contents, weather_objects[3]))||
			(weather_object_exists(troom->contents, weather_objects[4])))
		{
			//remove rain from room
			if((obj = get_obj_in_list_num (weather_objects[3], troom->contents)))
				extract_obj(obj);
			if((obj = get_obj_in_list_num (weather_objects[4], troom->contents)))
				extract_obj(obj);

			//load slush in room
			obj = load_object(weather_objects[2]);
			obj_to_room (obj, troom->nVirtual);
		}
		else
		{
			//if there is ash, replace it with muddy ash
			if((weather_object_exists(troom->contents, weather_objects[5]))||
				(weather_object_exists(troom->contents, weather_objects[6])))
			{
				//remove ash
				if((obj = get_obj_in_list_num (weather_objects[5], troom->contents)))
					extract_obj(obj);
				if((obj = get_obj_in_list_num (weather_objects[6], troom->contents)))
					extract_obj(obj);

				//if not already muddy ash in the room load some
				if(!(obj = get_obj_in_list_num (weather_objects[9], troom->contents)))
				{
					obj = load_object(weather_objects[9]);
					obj_to_room (obj, troom->nVirtual);
				}
			}
			else
			{
				//if there is old snow replace it with new snow
				if(weather_object_exists(troom->contents, weather_objects[1]))
				{
					//remove old snow from room
					if((obj = get_obj_in_list_num (weather_objects[1], troom->contents)))
						extract_obj(obj);

					//load fresh snow in room
					obj = load_object(weather_objects[0]);
					obj_to_room (obj, troom->nVirtual);
				}
				else
				{
					//if there is no snow or slush or muddy ash load fresh snow
					if((!weather_object_exists(troom->contents, weather_objects[2]))&&
						(!weather_object_exists(troom->contents, weather_objects[0]))&&
						(!weather_object_exists(troom->contents, weather_objects[9])))
					{
						//load fresh snow in room
						obj = load_object(weather_objects[0]);
						obj_to_room (obj, troom->nVirtual);
					}
				}
			}
		}
	}

	/* If volcanic smoke is set */
	if(weather_info[troom->zone].special_effect == VOLCANIC_SMOKE)
	{
		//if there is snow or rain or slush, replace it with muddy ash
		if((weather_object_exists(troom->contents, weather_objects[0]))||
			(weather_object_exists(troom->contents, weather_objects[1]))||
			(weather_object_exists(troom->contents, weather_objects[2]))||
			(weather_object_exists(troom->contents, weather_objects[3]))||
			(weather_object_exists(troom->contents, weather_objects[4])))
		{
				//remove snow/rain/slush
				if((obj = get_obj_in_list_num (weather_objects[0], troom->contents)))
					extract_obj(obj);
				if((obj = get_obj_in_list_num (weather_objects[1], troom->contents)))
					extract_obj(obj);
				if((obj = get_obj_in_list_num (weather_objects[2], troom->contents)))
					extract_obj(obj);
				if((obj = get_obj_in_list_num (weather_objects[3], troom->contents)))
					extract_obj(obj);
				if((obj = get_obj_in_list_num (weather_objects[4], troom->contents)))
					extract_obj(obj);

				//if not already muddy ash in the room load some
				if(!(obj = get_obj_in_list_num (weather_objects[9], troom->contents)))
				{
					obj = load_object(weather_objects[9]);
					obj_to_room (obj, troom->nVirtual);
				}
		}
		else
		{
			//if there is old ash replace it with new ash
			if(weather_object_exists(troom->contents, weather_objects[6]))
			{
				//remove old ash from room
				if((obj = get_obj_in_list_num (weather_objects[6], troom->contents)))
					extract_obj(obj);

				//load fresh ash in room
				obj = load_object(weather_objects[5]);
				obj_to_room (obj, troom->nVirtual);
			}
			else
			{
				//if there is no ash or muddy ash load it
				if((!weather_object_exists(troom->contents, weather_objects[5]))&&
					(!weather_object_exists(troom->contents, weather_objects[9])))
				{
					//load fresh ash in room
					obj = load_object(weather_objects[5]);
					obj_to_room (obj, troom->nVirtual);
				}
			}
		}
	}

	/* If foul stench is set */
	if(weather_info[troom->zone].special_effect == FOUL_STENCH)
	{
		//if there is old stench replace it with new stench
		if(weather_object_exists(troom->contents, weather_objects[8]))
		{
			//remove old stench from room
			if((obj = get_obj_in_list_num (weather_objects[8], troom->contents)))
				extract_obj(obj);

			//load fresh stench in room
			obj = load_object(weather_objects[7]);
			obj_to_room (obj, troom->nVirtual);
		}
		else
		{
			//if there is no stench load it
			if((!weather_object_exists(troom->contents, weather_objects[7])))
			{
				//load fresh stench in room
				obj = load_object(weather_objects[7]);
				obj_to_room (obj, troom->nVirtual);
			}
		}
	}
}

