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

#include "hphp/tools/tc-print/std-logger.h"
#include <stdarg.h>
#include <sstream>

namespace HPHP {

void StdLogger::printGeneric(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

void StdLogger::printBytecode(std::string byteInfo){
  std::cout<< byteInfo;
}

void StdLogger::printLine(std::string lineInfo) {
  std::cout << lineInfo;
}

void StdLogger::printAsm(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

bool StdLogger::flushTranslation(std::string, bool transOpt) {
  return true;
}

} // namespace HPHP
