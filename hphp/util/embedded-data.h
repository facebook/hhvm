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

#ifndef EMBEDDED_REPO_H
#define EMBEDDED_REPO_H

#include <string>

namespace HPHP {

struct embedded_data {
  std::string   m_filename;
  uint64_t      m_start;
  uint64_t      m_len;
#if (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))
  void*         m_handle;
#endif
};

bool get_embedded_data(const char *section, embedded_data* desc,
                       const std::string &filename = "");

}

#endif
