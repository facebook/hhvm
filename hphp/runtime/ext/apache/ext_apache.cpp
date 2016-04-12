/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/apache/ext_apache.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/util/health-monitor-types.h"
#include "hphp/util/text-util.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(apache_note, const String& note_name,
                      const String& note_value /* = null_string */) {
  String prev = ServerNote::Get(note_name);
  if (!note_value.empty()) {
    ServerNote::Add(note_name, note_value);
  }
  if (!prev.isNull()) {
    return prev;
  }
  return false;
}

static Array get_headers(const HeaderMap& headers, bool allHeaders = false) {
  ArrayInit ret(headers.size(), ArrayInit::Mixed{});
  for (auto& iter : headers) {
    const auto& values = iter.second;
    if (auto size = values.size()) {
      if (!allHeaders) {
        ret.set(String(iter.first), String(values.back()));
      } else {
        PackedArrayInit dups(size);
        for (auto& dup : values) {
          dups.append(String(dup));
        }
        ret.set(String(toLower(iter.first)), dups.toArray());
      }
    }
  }
  return ret.toArray();
}

Array HHVM_FUNCTION(apache_request_headers) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getHeaders(headers);
    return get_headers(headers);
  }
  return empty_array();
}

Array HHVM_FUNCTION(apache_response_headers) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getResponseHeaders(headers);
    return get_headers(headers);
  }
  return empty_array();
}

bool HHVM_FUNCTION(apache_setenv, const String& variable, const String& value,
                     bool walk_to_top /* = false */) {
  return false;
}

const StaticString
  s_restart_time("restart_time"),
  s_max_clients("max_clients"),
  s_active_clients("active_clients"),
  s_queued_requests("queued_requests"),
  s_child_status("child_status"),
  s_health_level("health_level");

HealthLevel ApacheExtension::m_healthLevel(HealthLevel::Bold);

Array HHVM_FUNCTION(apache_get_config) {
  int workers = 0, queued = 0, health_level = 0;
  if (HttpServer::Server) {
    workers = HttpServer::Server->getPageServer()->getActiveWorker();
    queued = HttpServer::Server->getPageServer()->getQueuedJobs();
    health_level = (int)(ApacheExtension::GetHealthLevel());
  }

  return make_map_array(
    s_restart_time, HttpServer::StartTime,
    s_max_clients, RuntimeOption::ServerThreadCount,
    s_active_clients, workers,
    s_queued_requests, queued,
    s_health_level, health_level
  );
}

Array HHVM_FUNCTION(get_headers_secure) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getHeaders(headers);
    return get_headers(headers, true);
  }
  return empty_array();
}

///////////////////////////////////////////////////////////////////////////////

bool ApacheExtension::Enable(true);

ApacheExtension::ApacheExtension() : Extension("apache") {}

ApacheExtension::~ApacheExtension() {}

void ApacheExtension::moduleInit() {
  if (Enable) {
    HHVM_FE(apache_note);
    HHVM_FE(apache_request_headers);
    HHVM_FE(apache_response_headers);
    HHVM_FE(apache_setenv);
    HHVM_FALIAS(getallheaders, apache_request_headers);
    HHVM_FE(apache_get_config);
    HHVM_FALIAS(HH\\get_headers_secure, get_headers_secure);

    HHVM_RC_INT(APACHE_MAP, 200);

    loadSystemlib();
  }
}

void ApacheExtension::moduleLoad(const IniSetting::Map& ini, Hdf config) {
  Enable = RuntimeOption::ServerExecutionMode() ||
           Config::GetBool(ini,
                           config, "Apache.EnableInCLI",
                           RuntimeOption::EnableHipHopSyntax);
}

static ApacheExtension s_apache_extension;

///////////////////////////////////////////////////////////////////////////////
}
