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

#include "hphp/util/optional.h"

#include <folly/Range.h>
#include <folly/container/F14Map.h>
#include <time.h>

/*
 * Request tracing facilitates execution tracing of the internal hhvm overhead
 * from bytecode compilation and JIT compilation. Data from this module is
 * exposed to the active request via an HNI extension, external clients via the
 * admin port, and to third-party extensions via a pluggable API.
 *
 * The admin server and ext_request-trace will expose total time spent and total
 * number of hits for each distinct event name across the lifetime of the
 * process or request, respectively.
 *
 * Timing and counts for scopes will be exposed as a meta-event via these
 * interfaces and will include the total time for which the scope was active.
 *
 * Scopes can be used to elaborate the names of events which they contain when
 * exposed via the admin server and extension, as well as to add attributes and
 * additional information to all contained events (which will be accessible via
 * the extensible interface).
 */
namespace HPHP { namespace rqtrace {

struct EventStats { uint64_t total_duration; uint64_t total_count; };
using EventMap = folly::F14FastMap<std::string, EventStats>;
using AnnotationMap = folly::F14FastMap<std::string, std::string>;

////////////////////////////////////////////////////////////////////////////////

struct Event {
  explicit Event(folly::StringPiece name);

  Event(const Event&) = delete;
  Event(Event&&) = default;
  Event& operator=(const Event&) = delete;
  Event& operator=(Event&&) = default;

  folly::StringPiece name() const;
  uint64_t startMicro() const;
  uint64_t stopMicro() const;
  uint64_t duration() const;
  const AnnotationMap& annotations() const;
  bool finished() const;

  void begin(Optional<timespec>);
  void end(Optional<timespec>);
  void annotate(folly::StringPiece key, folly::StringPiece value);

private:
  std::string m_name;
  uint64_t m_startMicro{0};
  uint64_t m_stopMicro{0};
  AnnotationMap m_annot;

  friend struct Scope;
};

struct Scope : Event {
  using Event::Event;

  const std::vector<Event>& events() const;
  void setEventPrefix(folly::StringPiece);
  void setEventSuffix(folly::StringPiece);

private:
  std::string m_prefix;
  std::string m_suffix;
  std::vector<Event> m_sortedEvents;

  Event& appendEvent(Event&& ev);
  friend struct Trace;
};

struct Trace final {
  Trace() = default;
  Trace(const Trace&) = delete;
  Trace(Trace&&) = default;
  Trace& operator=(const Trace&) = delete;
  Trace& operator=(Trace&&) = default;

  Scope& scope();
  const Scope& scope() const;

  EventStats stats_for(folly::StringPiece name) const;
  const EventMap& stats() const;
  const std::vector<Scope>& scopes() const;

private:
  Scope& createScope(folly::StringPiece name);
  void appendEvent(Event&&);
  void finishScope();
  bool hasActiveScope() const;

  EventMap m_stats;
  std::vector<Scope> m_sortedScopes;

  friend struct EventGuard;
  friend struct ScopeGuard;
};

////////////////////////////////////////////////////////////////////////////////

struct EventGuard final {
  explicit EventGuard(
    folly::StringPiece name,
    Optional<timespec> = std::nullopt
  );
  EventGuard(
    Trace* t,
    folly::StringPiece name,
    Optional<timespec> = std::nullopt
  );
  ~EventGuard();

  EventGuard(const EventGuard&) = delete;
  EventGuard(EventGuard&&) = delete;

  EventGuard& operator=(const EventGuard&) = delete;
  EventGuard& operator=(EventGuard&&) = delete;

  void annotate(folly::StringPiece key, folly::StringPiece value);
  void finish(Optional<timespec> = std::nullopt);

private:
  Trace* m_trace;
  Event m_event;
};

struct ScopeGuard final {
  explicit ScopeGuard(
    folly::StringPiece name,
    Optional<timespec> = std::nullopt
  );
  ScopeGuard(
    Trace* t,
    folly::StringPiece name,
    Optional<timespec> = std::nullopt
  );
  ~ScopeGuard();

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard(ScopeGuard&&) = delete;

  ScopeGuard& operator=(const ScopeGuard&) = delete;
  ScopeGuard& operator=(ScopeGuard&&) = delete;

  void annotate(folly::StringPiece key, folly::StringPiece value);
  void setEventPrefix(folly::StringPiece);
  void setEventSuffix(folly::StringPiece);
  void finish(Optional<timespec> = std::nullopt);

private:
  Trace* m_trace;
  Scope* m_scope;
};

////////////////////////////////////////////////////////////////////////////////

struct DisableTracing final {
  DisableTracing();
  ~DisableTracing();

  DisableTracing(const DisableTracing&) = delete;
  DisableTracing(DisableTracing&&) = delete;

  DisableTracing& operator=(const DisableTracing&) = delete;
  DisableTracing& operator=(DisableTracing&&) = delete;

private:
  Trace* m_trace;
};

////////////////////////////////////////////////////////////////////////////////

struct Range final {
  Range(folly::StringPiece, uint64_t, uint64_t, const Scope&, const Event&);

  folly::StringPiece name() const;
  uint64_t begin() const;
  uint64_t end() const;
  uint64_t duration() const;

  template<class F> void visitAnnotations(F&&) const;

private:
  std::string m_name;
  uint64_t m_startMicro;
  uint64_t m_stopMicro;

  const Scope& m_scope;
  const Event& m_event;
};

template<class F>
void visit_ranges(const Trace&, F&&);

////////////////////////////////////////////////////////////////////////////////

void record_trace(Trace&&);
void register_trace_hook(std::function<void(const Trace&)>);

EventStats process_stats_for(folly::StringPiece);

template<class F>
void visit_process_stats(F&&);

////////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/base/request-tracing-inl.h"
