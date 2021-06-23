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

#pragma once

#include <folly/AtomicHashMap.h>
#include <folly/MapUtil.h>
#include <folly/portability/SysTime.h>

#include "hphp/util/assertions.h"

namespace HPHP { struct StringData; }

namespace HPHP { namespace rqtrace {

namespace detail {

struct AtomicEventStats {
  struct Empty{};
  AtomicEventStats() = default;
  AtomicEventStats(Empty) {}

  operator EventStats() {
    return {
      total_duration.load(std::memory_order_relaxed),
      total_count.load(std::memory_order_relaxed)
    };
  }

  std::atomic<uint64_t> total_duration;
  std::atomic<uint64_t> total_count;
};

using AtomicEventMap = folly::AtomicHashMap<const StringData*,AtomicEventStats>;
extern AtomicEventMap g_events;

inline int64_t to_micros(timespec t) {
  return (int64_t(t.tv_sec) * 1000000) + t.tv_nsec / 1000;
}

inline int64_t CurrentMicroTime() {
  timeval tv;
  gettimeofday(&tv, 0);
  return (int64_t(tv.tv_sec) * 1000000) + tv.tv_usec;
}

////////////////////////////////////////////////////////////////////////////////
}

inline Event::Event(folly::StringPiece name) : m_name(name) {}

inline folly::StringPiece Event::name() const {
  return m_name;
}

inline uint64_t Event::startMicro() const {
  assertx(finished());
  return m_startMicro;
}

inline uint64_t Event::stopMicro() const {
  assertx(finished());
  return m_stopMicro;
}

inline uint64_t Event::duration() const {
  assertx(finished());
  return m_stopMicro - m_startMicro;
}

inline const AnnotationMap& Event::annotations() const {
  return m_annot;
}

inline bool Event::finished() const {
  return m_stopMicro && m_stopMicro;
}

inline void Event::begin(Optional<timespec> t) {
  assertx(!m_startMicro);
  m_startMicro = detail::CurrentMicroTime();
  m_startMicro = t ? detail::to_micros(*t) : detail::CurrentMicroTime();
}

inline void Event::end(Optional<timespec> t) {
  assertx(m_startMicro && !m_stopMicro);
  m_stopMicro = t ? detail::to_micros(*t) : detail::CurrentMicroTime();
}

inline void Event::annotate(folly::StringPiece key, folly::StringPiece value) {
  m_annot.emplace(key, value);
}

////////////////////////////////////////////////////////////////////////////////

inline const std::vector<Event>& Scope::events() const {
  return m_sortedEvents;
}

inline void Scope::setEventPrefix(folly::StringPiece s) {
  assertx(m_prefix.empty());
  m_prefix = std::string(s);
}

inline void Scope::setEventSuffix(folly::StringPiece s) {
  assertx(m_suffix.empty());
  m_suffix = std::string(s);
}

inline Event& Scope::appendEvent(Event&& ev) {
  assertx(!finished());
  assertx(ev.finished());
  assertx(
    m_sortedEvents.empty() ||
    m_sortedEvents.back().startMicro() + m_sortedEvents.back().duration()
      <= ev.startMicro()
  );
  ev.m_name.insert(ev.m_name.begin(), m_prefix.begin(), m_prefix.end());
  ev.m_name.append(m_suffix);
  m_sortedEvents.emplace_back(std::move(ev));
  return m_sortedEvents.back();
}

////////////////////////////////////////////////////////////////////////////////

inline Scope& Trace::createScope(folly::StringPiece name) {
  assertx(m_sortedScopes.empty() || m_sortedScopes.back().finished());
  m_sortedScopes.emplace_back(name);
  return m_sortedScopes.back();
}

inline void Trace::appendEvent(Event&& e) {
  assertx(!m_sortedScopes.empty());
  auto& new_ev = m_sortedScopes.back().appendEvent(std::move(e));
  auto& info = m_stats[std::string(new_ev.name())];
  info.total_duration += new_ev.duration();
  info.total_count++;
}

inline Scope& Trace::scope() {
  assertx(!m_sortedScopes.empty());
  assertx(!m_sortedScopes.back().finished());
  return m_sortedScopes.back();
}

inline const Scope& Trace::scope() const {
  return const_cast<Trace*>(this)->scope();
}

inline EventStats Trace::stats_for(folly::StringPiece name) const {
  return folly::get_default(m_stats, std::string(name), EventStats{});
}

inline const EventMap& Trace::stats() const {
  return m_stats;
}

inline void Trace::finishScope() {
  assertx(!m_sortedScopes.empty());
  assertx(m_sortedScopes.back().finished());

  auto& info = m_stats[std::string(m_sortedScopes.back().name())];
  info.total_duration += m_sortedScopes.back().duration();
  info.total_count++;
}

inline bool Trace::hasActiveScope() const {
  return !m_sortedScopes.empty() && !m_sortedScopes.back().finished();
}

////////////////////////////////////////////////////////////////////////////////

inline EventGuard::EventGuard(
  Trace* t,
  folly::StringPiece name,
  Optional<timespec> ts
) : m_trace(t && t->hasActiveScope() ? t : nullptr)
  , m_event(name)
{
  if (m_trace) m_event.begin(ts);
}

inline EventGuard::~EventGuard() {
  finish();
}

inline void EventGuard::annotate(
  folly::StringPiece key,
  folly::StringPiece value
) {
  if (m_trace) m_event.annotate(key, value);
}

inline void EventGuard::finish(Optional<timespec> t) {
  if (m_trace) {
    m_event.end(t);
    m_trace->appendEvent(std::move(m_event));
    m_trace = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////

inline ScopeGuard::ScopeGuard(
  Trace* t,
  folly::StringPiece name,
  Optional<timespec> ts
) : m_trace(t)
  , m_scope(t ? &t->createScope(name) : nullptr)
{
  if (m_scope) m_scope->begin(ts);
}

inline ScopeGuard::~ScopeGuard() {
  finish();
}

inline void ScopeGuard::annotate(folly::StringPiece k, folly::StringPiece v) {
  if (m_scope) m_scope->annotate(k, v);
}

inline void ScopeGuard::setEventPrefix(folly::StringPiece fx) {
  if (m_scope) m_scope->setEventPrefix(fx);
}

inline void ScopeGuard::setEventSuffix(folly::StringPiece fx) {
  if (m_scope) m_scope->setEventSuffix(fx);
}

inline void ScopeGuard::finish(Optional<timespec> t) {
  if (m_scope) {
    m_scope->end(t);
    m_trace->finishScope();
    m_trace = nullptr;
    m_scope = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////

inline Range::Range(
  folly::StringPiece name,
  uint64_t start,
  uint64_t end,
  const Scope& scope,
  const Event& event
) : m_name(name)
  , m_startMicro(start)
  , m_stopMicro(end)
  , m_scope(scope)
  , m_event(event)
{
  assertx(m_startMicro && m_stopMicro);
  assertx(m_stopMicro >= m_startMicro);
  assertx(!m_name.empty());
}

inline folly::StringPiece Range::name() const {
  return m_name;
}

inline uint64_t Range::begin() const {
  return m_startMicro;
}

inline uint64_t Range::end() const {
  return m_stopMicro;
}

inline uint64_t Range::duration() const {
  return m_stopMicro - m_startMicro;
}

template<class F>
void Range::visitAnnotations(F&& fun) const {
  for (auto& a : m_scope.annotations()) {
    fun(a.first, a.second);
  }

  if ((Event*)&m_scope == &m_event) return;

  for (auto& a : m_event.annotations()) {
    fun(a.first, a.second);
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class F>
void visit_ranges(const Trace& t, F&& fun) {
  for (auto& s : t.scopes()) {
    if (s.events().empty()) {
      fun(Range(
        folly::sformat("{}_BEGIN to {}_END", s.name(), s.name()),
        s.startMicro(),
        s.stopMicro(),
        s,
        s
      ));
      continue;
    }
    fun(Range(
      folly::sformat("{}_BEGIN to {}", s.name(), s.events().front().name()),
      s.startMicro(),
      s.events().front().startMicro(),
      s,
      s
    ));
    for (int i = 0; i < s.events().size() - 1; ++i) {
      auto const& start = s.events()[i];
      auto const& end = s.events()[i];
      fun(Range(
        folly::sformat("{} to {}", start.name(), end.name()),
        start.startMicro(),
        start.stopMicro(),
        s,
        start
      ));
    }
    fun(
      folly::sformat("{} to {}_END", s.events().back().name(), s.name()),
      s.events().back().startMicro(),
      s.stopMicro(),
      s,
      s.events().back()
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class F>
void visit_process_stats(F&& fun) {
  for (auto& pair : detail::g_events) {
    fun(pair.first, pair.second);
  }
}

////////////////////////////////////////////////////////////////////////////////
}}

