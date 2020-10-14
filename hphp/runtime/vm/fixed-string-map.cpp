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

#include "hphp/runtime/vm/fixed-string-map.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/struct-log-util.h"

#include "hphp/util/stack-trace.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace FSM {

const StringData* null_key;

bool is_in_request_context() {
  return !g_context.isNull();
}

void log_to_scuba(const StringData* key_in_hashtable,
                  const StringData* key_for_query) {
  auto const rate = RO::EvalRaiseOnCaseInsensitiveLookupSampleRate;
  if (!StructuredLog::coinflip(rate)) return;
  StructuredLogEntry sample;
  sample.setStr("key_in_hashtable", key_in_hashtable->data());
  sample.setStr("key_for_query", key_for_query->data());

  StackTrace st;
  sample.setStackTrace("stack", st);
  StructuredLog::log("hhvm_case_insensitive_access", sample);
}

} // FSM

///////////////////////////////////////////////////////////////////////////////
}
