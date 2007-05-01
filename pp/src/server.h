//////////////////////////////////////////////////////////////////////////////
//
/// server.h - Server Class Structures and Routines
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2006 C. W. McHenry
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

#ifndef _rpie_server_h_
#define _rpie_server_h_
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sys/time.h>
#include <sys/resource.h>

namespace rpie 
{
  class server
    {
    private:
      static const int BOOT_DB_ABORT_THRESHOLD = 15; ///< Infinite loop test.
      static const int RUNNING_ABORT_THRESHOLD = 5; ///< Infinite loop test.
      std::map<std::string,std::string> string_variables;
      std::map<std::string,int> int_variables;
      std::set<std::string> bool_variables; // (true if varname exists)

      bool abort_threshold_enabled;
      int abort_threshold;
      int last_checkpoint;

      int get_user_seconds ()
	{
	  struct rusage rus;
	  getrusage (RUSAGE_SELF, &rus);
	  return rus.ru_utime.tv_sec;
	}

    public:
      static const size_t MAX_NAME_LENGTH = 15;	///< Username string length
      static const int ALARM_FREQUENCY = 20; ///< ITimer frequency in seconds
      
      void enable_timer_abort ()
	{
	  abort_threshold_enabled = true;
	}
      void disable_timer_abort ()
	{
	  abort_threshold_enabled = false;
	}

      void set_abort_threshold_pre_booting () 
	{
	  abort_threshold_enabled = true;
	  abort_threshold = BOOT_DB_ABORT_THRESHOLD;
	  last_checkpoint = get_user_seconds ();
	}

      void set_abort_threshold_post_booting ()
	{
	  last_checkpoint = get_user_seconds ();

	  if (abort_threshold == BOOT_DB_ABORT_THRESHOLD)
	    {
	      abort_threshold = RUNNING_ABORT_THRESHOLD;
	    }
	}

      bool loop_detect ()
	{
	  int timeslice = get_user_seconds () - last_checkpoint;	  
	  return (abort_threshold_enabled 
		  && (timeslice > abort_threshold));
	}
      
      void set_config (std::string &var_name, bool var_value = true);
      void set_config (std::string &var_name, std::string &var_value);
      std::string& get_config (std::string& var_name);
      std::string& get_config (const char * var_name);
      void load_config_files ();
      void load_config_file (std::ifstream &config_file);
    };

}

#endif // _rpie_server_h_
