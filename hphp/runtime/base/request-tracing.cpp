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

#include "hphp/runtime/base/request-tracing.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP { namespace rqtrace {

namespace detail { AtomicEventMap g_events{128}; }
namespace { std::function<void(const Trace&)> s_hook; }

void record_trace(Trace&& t) {
  if (s_hook) s_hook(t);

  for (auto& s : t.stats()) {
    auto const key = makeStaticString(s.first);
    auto const result = detail::g_events.emplace(
      key, detail::AtomicEventStats::Empty{});
    auto& stats = result.first->second;
    stats.total_count += s.second.total_count;
    stats.total_duration += s.second.total_duration;
  }
}

void register_trace_hook(std::function<void(const Trace&)> h) { s_hook = h; }

EventStats process_stats_for(folly::StringPiece k) {
  if (auto key = lookupStaticString(String(k).get())) {
    auto it = detail::g_events.find(key);
    if (it != detail::g_events.end()) return it->second;
  }
  return {};
}

namespace {
Trace* activeTrace() {
  return !g_context.isNull() ? g_context->getRequestTrace() : nullptr;
}
}

ScopeGuard::ScopeGuard(folly::StringPiece name,Optional<timespec> t)
  : ScopeGuard(activeTrace(), name, t)
{}

EventGuard::EventGuard(folly::StringPiece name,Optional<timespec> t)
  : EventGuard(activeTrace(), name, t)
{}

DisableTracing::DisableTracing()
  : m_trace(activeTrace())
{
  if (!g_context.isNull()) g_context->setRequestTrace(nullptr);
}

DisableTracing::~DisableTracing() {
  if (m_trace) g_context->setRequestTrace(m_trace);
}

}}
