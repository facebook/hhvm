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
#ifndef incl_HPHP_EXCEPTIONS_H_
#define incl_HPHP_EXCEPTIONS_H_

#include <string>
#include <cstdarg>

#include "hphp/util/portability.h"
#include "hphp/util/exception.h"
#include "hphp/runtime/base/exceptions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct AnalysisTimeFatalException : Exception {
  AnalysisTimeFatalException(const std::string& file,
                             int line,
                             const char* msg, ...) ATTRIBUTE_PRINTF(4,5)
    : m_file(file)
    , m_line(line)
  {
    va_list ap;
    va_start(ap, msg);
    format(msg, ap);
    va_end(ap);
  }

  EXCEPTION_COMMON_IMPL(AnalysisTimeFatalException);

  std::string m_file;
  int m_line;
};

//////////////////////////////////////////////////////////////////////

}

#endif
