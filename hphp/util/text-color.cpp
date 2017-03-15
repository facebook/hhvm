/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/text-color.h"

#include "hphp/util/text-util.h"

#include <cstring>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char *s_stdout_color = nullptr;
const char *s_stderr_color = nullptr;

const char *get_color_by_name(const std::string &name) {
  auto upper = toUpper(name);
  if (upper == "BLACK"         ) return ANSI_COLOR_BLACK;
  if (upper == "RED"           ) return ANSI_COLOR_RED;
  if (upper == "GREEN"         ) return ANSI_COLOR_GREEN;
  if (upper == "BROWN"         ) return ANSI_COLOR_BROWN;
  if (upper == "BLUE"          ) return ANSI_COLOR_BLUE;
  if (upper == "MAGENTA"       ) return ANSI_COLOR_MAGENTA;
  if (upper == "CYAN"          ) return ANSI_COLOR_CYAN;
  if (upper == "GRAY"          ) return ANSI_COLOR_GRAY;
  if (upper == "DARK_GRAY"     ) return ANSI_COLOR_DARK_GRAY;
  if (upper == "LIGHT_RED"     ) return ANSI_COLOR_LIGHT_RED;
  if (upper == "LIGHT_GREEN"   ) return ANSI_COLOR_LIGHT_GREEN;
  if (upper == "YELLOW"        ) return ANSI_COLOR_YELLOW;
  if (upper == "LIGHT_BLUE"    ) return ANSI_COLOR_LIGHT_BLUE;
  if (upper == "LIGHT_MAGENTA" ) return ANSI_COLOR_LIGHT_MAGENTA;
  if (upper == "LIGHT_CYAN"    ) return ANSI_COLOR_LIGHT_CYAN;
  if (upper == "WHITE"         ) return ANSI_COLOR_WHITE;
  return nullptr;
}

const char *get_bgcolor_by_name(const std::string &name) {
  auto upper = toUpper(name);
  if (upper == "BLACK"         ) return  ANSI_BGCOLOR_BLACK;
  if (upper == "RED"           ) return  ANSI_BGCOLOR_RED;
  if (upper == "GREEN"         ) return  ANSI_BGCOLOR_GREEN;
  if (upper == "BROWN"         ) return  ANSI_BGCOLOR_BROWN;
  if (upper == "BLUE"          ) return  ANSI_BGCOLOR_BLUE;
  if (upper == "MAGENTA"       ) return  ANSI_BGCOLOR_MAGENTA;
  if (upper == "CYAN"          ) return  ANSI_BGCOLOR_CYAN;
  if (upper == "GRAY"          ) return  ANSI_BGCOLOR_GRAY;
  return nullptr;
}

void get_supported_colors(std::vector<std::string> &names) {
  names.push_back("black"         );
  names.push_back("red"           );
  names.push_back("green"         );
  names.push_back("brown"         );
  names.push_back("blue"          );
  names.push_back("magenta"       );
  names.push_back("cyan"          );
  names.push_back("gray"          );
  names.push_back("dark_gray"     );
  names.push_back("light_red"     );
  names.push_back("light_green"   );
  names.push_back("yellow"        );
  names.push_back("light_blue"    );
  names.push_back("light_magenta" );
  names.push_back("light_cyan"    );
  names.push_back("white"         );
}

std::string add_bgcolor(const char *color, const char *bgcolor) {
  assert(color && *color && color[strlen(color) - 1] == 'm');
  assert(bgcolor && *bgcolor);

  std::string ret = color;
  return ret.substr(0, ret.length() - 1) + bgcolor;
}

///////////////////////////////////////////////////////////////////////////////
}
