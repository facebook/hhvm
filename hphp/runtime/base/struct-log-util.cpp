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

#include "hphp/runtime/base/struct-log-util.h"

#include "hphp/util/stack-trace.h"

namespace HPHP { namespace StructuredLog {

void logSerDes(const char* format, const char* op,
               const String& serialized, const Variant& value) {
  StructuredLogEntry sample;
  sample.setStr("format", format);
  sample.setStr("operation", op);
  DataType t = value.getType();
  sample.setStr("type", tname(t));
  if (isArrayType(t)) {
    sample.setInt("array_len", value.asCArrRef().length());
  }
  if (auto sd = serialized.get()) {
    sample.setInt("length_ser", sd->size());
    sample.setInt("hash_ser", sd->hash());
  }
  StackTrace st;
  sample.setStackTrace("stack", st);
  StructuredLog::log("hhvm_serdes", sample);
}

}}
