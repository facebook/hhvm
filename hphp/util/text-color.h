/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_UTIL_TEXT_COLOR_H_
#define incl_HPHP_UTIL_TEXT_COLOR_H_

#include <vector>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char ANSI_COLOR_BLACK[]          = "\033[22;30m";
const char ANSI_COLOR_RED[]            = "\033[22;31m";
const char ANSI_COLOR_GREEN[]          = "\033[22;32m";
const char ANSI_COLOR_BROWN[]          = "\033[22;33m";
const char ANSI_COLOR_BLUE[]           = "\033[22;34m";
const char ANSI_COLOR_MAGENTA[]        = "\033[22;35m";
const char ANSI_COLOR_CYAN[]           = "\033[22;36m";
const char ANSI_COLOR_GRAY[]           = "\033[22;37m";

const char ANSI_COLOR_DARK_GRAY[]      = "\033[01;30m";
const char ANSI_COLOR_LIGHT_RED[]      = "\033[01;31m";
const char ANSI_COLOR_LIGHT_GREEN[]    = "\033[01;32m";
const char ANSI_COLOR_YELLOW[]         = "\033[01;33m";
const char ANSI_COLOR_LIGHT_BLUE[]     = "\033[01;34m";
const char ANSI_COLOR_LIGHT_MAGENTA[]  = "\033[01;35m";
const char ANSI_COLOR_LIGHT_CYAN[]     = "\033[01;36m";
const char ANSI_COLOR_WHITE[]          = "\033[01;37m";

const char ANSI_BGCOLOR_BLACK[]        = ";40m";
const char ANSI_BGCOLOR_RED[]          = ";41m";
const char ANSI_BGCOLOR_GREEN[]        = ";42m";
const char ANSI_BGCOLOR_BROWN[]        = ";43m";
const char ANSI_BGCOLOR_BLUE[]         = ";44m";
const char ANSI_BGCOLOR_MAGENTA[]      = ";45m";
const char ANSI_BGCOLOR_CYAN[]         = ";46m";
const char ANSI_BGCOLOR_GRAY[]         = ";47m";

const char ANSI_COLOR_END[]            = "\033[0m";

extern const char *s_stdout_color;
extern const char *s_stderr_color;

void get_supported_colors(std::vector<std::string> &names);

const char *get_color_by_name(const std::string &name);
const char *get_bgcolor_by_name(const std::string &name);
std::string add_bgcolor(const char *color, const char *bgcolor);

///////////////////////////////////////////////////////////////////////////////
}

#endif
