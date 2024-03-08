/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

void HHVM_FUNCTION(apache_notes, const Array& notes) {
  ServerNote::AddNotes(notes);
}

Variant HHVM_FUNCTION(apache_note, const String& note_name,
                      const Variant& note_value /* = empty_string */) {
  String prev = ServerNote::Get(note_name);
  if (note_value.isNull()) {
    ServerNote::Delete(note_name);
  } else if (!note_value.isString()) {
    raise_warning("apache_note() expects parameter 2 to be a nullable string");
    return false;
  } else {
    auto const& value = note_value.asCStrRef();
    if (!value.empty()) {
      ServerNote::Add(note_name, value);
    }
  }
  if (!prev.isNull()) {
    return prev;
  }
  return false;
}

Array HHVM_FUNCTION(apache_request_headers) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    auto const& headers = transport->getHeaders();
    return get_headers(headers);
  }
  return empty_dict_array();
}

static Array get_raw_headers(const proxygen::HTTPHeaders &headers) {
  VecInit ret(headers.size());
  headers.forEach([&] (const std::string &header, const std::string &val) {
    auto headerValPair = make_vec_array(header.c_str(), val.c_str());
    ret.append(headerValPair);
  });
  return ret.toArray();
}

Array HHVM_FUNCTION(get_proxygen_headers) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    auto const headers = transport->getProxygenHeaders();
    if (headers) {
      return get_raw_headers(*headers);
    }
  }
  return empty_vec_array();
}

Array HHVM_FUNCTION(apache_response_headers) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    HeaderMap headers;
    transport->getResponseHeaders(headers);
    return get_headers(headers);
  }
  return empty_dict_array();
}

bool HHVM_FUNCTION(apache_setenv, const String& /*variable*/,
                   const String& /*value*/,
                   bool /*walk_to_top*/ /* = false */) {
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
    queued -= HttpServer::QueueDiscount.load(std::memory_order_relaxed);
    if (queued < 0)
      queued = 0;
    health_level = (int)(ApacheExtension::GetHealthLevel());
  }

  return make_dict_array(
    s_restart_time, HttpServer::StartTime,
    s_max_clients, Cfg::Server::ThreadCount,
    s_active_clients, workers,
    s_queued_requests, queued,
    s_health_level, health_level
  );
}

Array HHVM_FUNCTION(get_headers_secure) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    auto const& headers = transport->getHeaders();
    return get_headers(headers, true);
  }
  return empty_dict_array();
}

///////////////////////////////////////////////////////////////////////////////

ApacheExtension::ApacheExtension() : Extension("apache", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

ApacheExtension::~ApacheExtension() {}

void ApacheExtension::moduleRegisterNative() {
  HHVM_FE(apache_note);
  HHVM_FE(apache_notes);
  HHVM_FE(apache_request_headers);
  HHVM_FE(apache_response_headers);
  HHVM_FE(apache_setenv);
  HHVM_FALIAS(getallheaders, apache_request_headers);
  HHVM_FE(apache_get_config);
  HHVM_FALIAS(HH\\get_headers_secure, get_headers_secure);
  HHVM_FALIAS(HH\\get_proxygen_headers, get_proxygen_headers);
}

static ApacheExtension s_apache_extension;

///////////////////////////////////////////////////////////////////////////////
}
