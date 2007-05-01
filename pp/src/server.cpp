#include "server.h"

void 
rpie::server::set_config (std::string &var_name, bool var_value)
{
  if (var_value)
    {
      bool_variables.insert (var_name);
    }
}

void
rpie::server::set_config (std::string &var_name, std::string &var_value)
{
  if (var_value.size())
    {
      string_variables[var_name] = var_value;
    }
}

std::string&
rpie::server::get_config (std::string& var_name)
{
  return string_variables.find (var_name)->second;
}

std::string&
rpie::server::get_config (const char * var_name)
{
  return string_variables.find (var_name)->second;
}


/// Load configuration files from the usual places
void
rpie::server::load_config_files ()
{
  std::vector<std::string> config_filenames;
  
  /// Push back filenames.
  config_filenames.push_back ("./.rpi_engine/config");
  config_filenames.push_back ("~/.rpi_engine/config");
  config_filenames.push_back ("/etc/rpi_engine/config");
  typedef std::vector<std::string>::const_iterator VI;
  for (VI i = config_filenames.begin ();
       i != config_filenames.end (); ++i)
    {
      std::ifstream config_file ((*i).c_str());
      if (config_file.is_open ())
	{
	  load_config_file (config_file);
	}
    }
}

/// Get the db connection info from the configuration file
void
rpie::server::load_config_file (std::ifstream &config_file)
{
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
      
      ///\TODO Make a more robust parser
      
      // we place delimiter '=' first so we can skip it easy
      const char * const delimiters = "= ;\t\r\n";
      std::string var_name;
      
      // skip whitespace to start of variable
      std::string::size_type var_start = 
	line.find_first_not_of (delimiters+1);
      if (var_start == std::string::npos)
	{
	  continue;
	}
      
      // find end of variable name
      std::string::size_type var_end =
	line.find_first_of (delimiters, var_start);
      if (var_end == std::string::npos) 
	{
	  // add boolean: var_name with nothing else
	  var_name.assign (line.substr (var_start));
	  set_config (var_name,true);
	  continue;
	}
      else
	{
	  // store var_name for later
	  std::string::size_type len = var_end - var_start;
	  var_name.assign (line.substr (var_start, len));
	}

      // skip whitespace to start of '='
      std::string::size_type eq_start = 
	line.find_first_not_of (delimiters+1, var_end);
      if (eq_start == std::string::npos)
	{
	  // add boolean: var_name with nothing else
	  set_config (var_name,true);
	  continue;
	}
      if (line.at(eq_start) != '=')
	{
	  std::cerr << "Warning, bad syntax on line: " 
		    << line << std::endl;
	  continue;
	}

      // skip whitespace to start of '='
      std::string::size_type val_start = 
	line.find_first_not_of (delimiters+1, eq_start+1);
      if (val_start == std::string::npos)
	{
	  std::cerr << "Warning, bad syntax on line: " 
		    << line << std::endl;
	  continue;
	}
      
      std::string val_string;
      char next_c = line.at(val_start);
      if (next_c == '"' || next_c == '\'')
	{
	  ++val_start;
	  std::string::size_type val_end = 
	    line.find_first_of (next_c, val_start);
	  
	  if (val_end == std::string::npos)
	    {
	      std::cerr << "Warning, bad syntax on line: " 
			<< line << std::endl;
	      continue;
	    }
		  
	  val_string = line.substr (val_start, val_end - val_start);
	  set_config (var_name, val_string);
	}
    }
}
