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
#include <sys/time.h>
#include <sys/resource.h>

namespace rpie 
{
  class server
    {
    private:
      static const int BOOT_DB_ABORT_THRESHOLD = 15; ///< Infinite loop test.
      static const int RUNNING_ABORT_THRESHOLD = 5; ///< Infinite loop test.
      std::string mysql_username;
      std::string mysql_password;

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

      /// Get the db connection info from the configuration file
      void load_config_file (std::string config_filename)
        {
          std::ifstream config_file (config_filename.c_str ());
          std::string line;
          while (std::getline (config_file,line))
            {
	      // Remove comments
	      std::string::size_type comment_start = line.find_first_of ('#');
	      if (comment_start != std::string::npos)
		{
		  line.erase (comment_start, std::string::npos);
		}
	      if (line.empty ())
		{
		  continue;
		}

	      if (line.find ('=') != std::string::npos)
		{
		  std::istringstream input_line (line);
		  std::string var;
		  std::string eq;
		  std::string value;
		  input_line >> var >> eq >> std::ws; 
		  std::getline (input_line, value);
		  if (eq.compare ("=") == 0)
		    {
		      std::cerr << var << '=' << value << std::endl;
		      if (var.compare ("mysql_username") == 0)
			{
			  if (value[0] == '"' || value[0] == '\'')
			    {
			      std::string::size_type start_quote, end_quote;
			      start_quote = value.find_first_of ("'\"");
			      end_quote = value.find_first_of ("'\"");
				  std::cerr << start_quote << ',' << value 
					    << ',' << end_quote << std::endl;
			      if (start_quote != end_quote 
				  && start_quote != std::string::npos
				  && end_quote != std::string::npos) 
				{
				  value.erase (start_quote);
				  value.erase (end_quote);
				  std::cerr << start_quote << ',' << value 
					    << ',' << end_quote << std::endl;
				}

			    }
			}
		    }
		}
	      
            }
        }

    };

}

#endif // _rpie_server_h_
