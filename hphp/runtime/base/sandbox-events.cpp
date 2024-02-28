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
#include "hphp/runtime/base/sandbox-events.h"
#include "hphp/runtime/base/configs/autoload.h"

namespace HPHP {

void logSboxEvent(uint32_t sample_rate, std::string_view source,
    std::string_view event, std::string_view key, uint64_t duration_us,
    HPHP::Optional<std::string_view> error_msg) {
  if (!getenv("INSIDE_RE_WORKER")) {
    StructuredLogEntry ent;
    ent.force_init = true;
    ent.setProcessUuid("hhvm_uuid");
    ent.setInt("sample_rate", sample_rate);
    ent.setStr("source", source);
    ent.setStr("event", event);
    ent.setStr("key", key);
    ent.setInt("duration_us", duration_us);
    if (error_msg.has_value()) {
      ent.setStr("error_what", error_msg.value());
      ent.setInt("error", 1);
    } else {
      ent.setInt("error", 0);
    }
    StructuredLog::log("hhvm_sandbox_events", ent);
  }
}

void rareSboxEvent(std::string_view source, std::string_view event,
                   std::string_view key) {
  if (Cfg::Autoload::PerfSampleRate != 0) {
    logSboxEvent(1, source, event, key, 0, std::nullopt);
  }
}

}
