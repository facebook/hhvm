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

#ifndef incl_HPHP_TCPRINT_LOGGER_H_
#define incl_HPHP_TCPRINT_LOGGER_H_
#include <iostream>

namespace HPHP {

struct TCPrintLogger {
  virtual ~TCPrintLogger() {};
  // Standard printing (usually to std::cout).
  virtual void printGeneric(const char* format, ...) = 0;
  // Printing of the collected bytecode at the start of translations.
  virtual void printBytecode(std::string byteInfo) = 0;
  // Printing of line information (and assoc bytecode).
  virtual void printLine(std::string lineInfo) = 0;
  // Printing of generated assembly.
  virtual void printAsm(const char* format, ...) = 0;
  // If relevant, pushing formatted output to database.
  virtual bool flushTranslation(std::string, bool transOpt) = 0;
};

} // namespace HPHP

#endif
