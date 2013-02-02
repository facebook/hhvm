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

#ifndef __UTIL_TEXT_COLOR_H__
#define __UTIL_TEXT_COLOR_H__

#include "base.h"

namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////

#define ANSI_COLOR_BLACK          "\033[22;30m"
#define ANSI_COLOR_RED            "\033[22;31m"
#define ANSI_COLOR_GREEN          "\033[22;32m"
#define ANSI_COLOR_BROWN          "\033[22;33m"
#define ANSI_COLOR_BLUE           "\033[22;34m"
#define ANSI_COLOR_MAGENTA        "\033[22;35m"
#define ANSI_COLOR_CYAN           "\033[22;36m"
#define ANSI_COLOR_GRAY           "\033[22;37m"

#define ANSI_COLOR_DARK_GRAY      "\033[01;30m"
#define ANSI_COLOR_LIGHT_RED      "\033[01;31m"
#define ANSI_COLOR_LIGHT_GREEN    "\033[01;32m"
#define ANSI_COLOR_YELLOW         "\033[01;33m"
#define ANSI_COLOR_LIGHT_BLUE     "\033[01;34m"
#define ANSI_COLOR_LIGHT_MAGENTA  "\033[01;35m"
#define ANSI_COLOR_LIGHT_CYAN     "\033[01;36m"
#define ANSI_COLOR_WHITE          "\033[01;37m"

#define ANSI_BGCOLOR_BLACK        ";40m"
#define ANSI_BGCOLOR_RED          ";41m"
#define ANSI_BGCOLOR_GREEN        ";42m"
#define ANSI_BGCOLOR_BROWN        ";43m"
#define ANSI_BGCOLOR_BLUE         ";44m"
#define ANSI_BGCOLOR_MAGENTA      ";45m"
#define ANSI_BGCOLOR_CYAN         ";46m"
#define ANSI_BGCOLOR_GRAY         ";47m"

#define ANSI_COLOR_END            "\033[0m"

extern const char *s_stdout_color;
extern const char *s_stderr_color;

void get_supported_colors(std::vector<std::string> &names);

const char *get_color_by_name(const char *name);
const char *get_bgcolor_by_name(const char *name);
std::string add_bgcolor(const char *color, const char *bgcolor);

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __UTIL_TEXT_COLOR_H__
