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

#ifndef incl_HPHP_STD_TCP_LOGGER_H_
#define incl_HPHP_STD_TCP_LOGGER_H_

#include "hphp/tools/tc-print/tc-print-logger.h"
#include <iostream>
#include <stdio.h>

namespace HPHP {

struct StdLogger : TCPrintLogger {
  void printGeneric(const char* format, ...) override;
  void printBytecode(std::string byteInfo) override;
  void printLine(std::string lineInfo) override;
  void printAsm(const char* format, ...) override;
  bool flushTranslation(std::string, bool transOpt) override;
};

} // namespace HPHP

#endif
