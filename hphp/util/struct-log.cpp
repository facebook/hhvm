/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 Facebook, Inc. (http://www.facebook.com)          |
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
#include "hphp/util/struct-log.h"

namespace HPHP {

StructuredLogImpl StructuredLog::s_impl = nullptr;

bool StructuredLog::enabled() {
  return s_impl != nullptr;
}

void StructuredLog::enable(StructuredLogImpl impl) {
  s_impl = impl;
}

void StructuredLog::log(const std::string& tableName,
                        const std::map<std::string, int64_t>& cols) {
  if (enabled()) {
    s_impl(tableName, cols);
  }
}

}
