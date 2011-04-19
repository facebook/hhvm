/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "text_color.h"
#include "util.h"

using namespace std;

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

const char *s_stdout_color = NULL;
const char *s_stderr_color = NULL;

const char *get_color_by_name(const char *name) {
  string upper = toUpper(name);
  if (upper == "BLACK"         ) return  ANSI_COLOR_BLACK;
  if (upper == "RED"           ) return  ANSI_COLOR_RED;
  if (upper == "GREEN"         ) return  ANSI_COLOR_GREEN;
  if (upper == "BROWN"         ) return  ANSI_COLOR_BROWN;
  if (upper == "BLUE"          ) return  ANSI_COLOR_BLUE;
  if (upper == "MAGENTA"       ) return  ANSI_COLOR_MAGENTA;
  if (upper == "CYAN"          ) return  ANSI_COLOR_CYAN;
  if (upper == "GRAY"          ) return  ANSI_COLOR_GRAY;
  if (upper == "DARK_GRAY"     ) return  ANSI_COLOR_DARK_GRAY;
  if (upper == "LIGHT_RED"     ) return  ANSI_COLOR_LIGHT_RED;
  if (upper == "LIGHT_GREEN"   ) return  ANSI_COLOR_LIGHT_GREEN;
  if (upper == "YELLOW"        ) return  ANSI_COLOR_YELLOW;
  if (upper == "LIGHT_BLUE"    ) return  ANSI_COLOR_LIGHT_BLUE;
  if (upper == "LIGHT_MAGENTA" ) return  ANSI_COLOR_LIGHT_MAGENTA;
  if (upper == "LIGHT_CYAN"    ) return  ANSI_COLOR_LIGHT_CYAN;
  if (upper == "WHITE"         ) return  ANSI_COLOR_WHITE;
  return NULL;
}

const char *get_bgcolor_by_name(const char *name) {
  string upper = toUpper(name);
  if (upper == "BLACK"         ) return  ANSI_BGCOLOR_BLACK;
  if (upper == "RED"           ) return  ANSI_BGCOLOR_RED;
  if (upper == "GREEN"         ) return  ANSI_BGCOLOR_GREEN;
  if (upper == "BROWN"         ) return  ANSI_BGCOLOR_BROWN;
  if (upper == "BLUE"          ) return  ANSI_BGCOLOR_BLUE;
  if (upper == "MAGENTA"       ) return  ANSI_BGCOLOR_MAGENTA;
  if (upper == "CYAN"          ) return  ANSI_BGCOLOR_CYAN;
  if (upper == "GRAY"          ) return  ANSI_BGCOLOR_GRAY;
  return NULL;
}

void get_supported_colors(std::vector<std::string> &names) {
  names.push_back(toLower("BLACK"         ));
  names.push_back(toLower("RED"           ));
  names.push_back(toLower("GREEN"         ));
  names.push_back(toLower("BROWN"         ));
  names.push_back(toLower("BLUE"          ));
  names.push_back(toLower("MAGENTA"       ));
  names.push_back(toLower("CYAN"          ));
  names.push_back(toLower("GRAY"          ));
  names.push_back(toLower("DARK_GRAY"     ));
  names.push_back(toLower("LIGHT_RED"     ));
  names.push_back(toLower("LIGHT_GREEN"   ));
  names.push_back(toLower("YELLOW"        ));
  names.push_back(toLower("LIGHT_BLUE"    ));
  names.push_back(toLower("LIGHT_MAGENTA" ));
  names.push_back(toLower("LIGHT_CYAN"    ));
  names.push_back(toLower("WHITE"         ));
}

std::string add_bgcolor(const char *color, const char *bgcolor) {
  ASSERT(color && *color && color[strlen(color) - 1] == 'm');
  ASSERT(bgcolor && *bgcolor);

  string ret = color;
  return ret.substr(0, ret.length() - 1) + bgcolor;
}

///////////////////////////////////////////////////////////////////////////////
}}
