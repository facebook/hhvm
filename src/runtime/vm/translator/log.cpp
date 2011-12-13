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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "log.h"

namespace HPHP {
namespace VM {
namespace Log {

#ifndef RELEASE /* { */

static FILE* out;

void initLogFile() {
  if (!out) {
    out = fopen("vm.log", "w");
    if (!out) {
      fprintf(stderr, "could not create log file\n");
      exit(1);
    }
  }
}

void vlog(const char *fmt, va_list args) {
  initLogFile();
  vfprintf(out, fmt, args);
  fflush(out);
}

void log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vlog(fmt, args);
  va_end(args);
}

#endif /* } */

} } }

