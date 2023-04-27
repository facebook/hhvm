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

#include <chrono>
#include <cassert>

#include <boost/variant.hpp>
#include <memory>

#include "hphp/hhbbc/options.h"

#include "hphp/runtime/vm/repo-global-data.h"

#include "hphp/util/compact-vector.h"
#include "hphp/util/extern-worker.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"
#include "hphp/util/tribool.h"

namespace HPHP {
struct StringData;
struct ArrayData;
}

namespace HPHP {

struct StructuredLogEntry;

namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * String that must be a static string, and Array that must be a
 * static array.
 */
using SString  = const StringData*;
using LSString = LowPtr<const StringData>;
using SArray   = const ArrayData*;

struct Bytecode;
using BytecodeVec = CompactVector<Bytecode>;

/*
 * HHBC evaluation stack flavors.
 */
enum class Flavor { C, U, CU };

/*
 * Types of parameter preparation (or unknown).
 */
struct PrepKind {
  TriBool inOut;
  TriBool readonly;
  bool operator==(const PrepKind& o) const {
    return std::tie(inOut, readonly) == std::tie(o.inOut, o.readonly);
  }
  size_t hash() const {
    return folly::hash::hash_combine(inOut, readonly);
  }
  template <typename SerDe> void serde(SerDe& sd) {
    sd(inOut)
      (readonly)
      ;
  }
};
using PrepKindVec = CompactVector<PrepKind>;

using LocalId = uint32_t;
constexpr const LocalId NoLocalId = -1;
/*
 * Special value used by StackElem::equivLoc to indicate that
 * this element is a dup of the one below.
 */
constexpr const LocalId StackDupId = -2;
constexpr const LocalId StackThisId = -3;
constexpr const LocalId MaxLocalId = StackThisId - 1;

using ClosureId = uint32_t;
using IterId = uint32_t;
using BlockId = uint32_t;
constexpr const BlockId NoBlockId = -1;
using ExnNodeId = uint32_t;
constexpr const ExnNodeId NoExnNodeId = -1;

//////////////////////////////////////////////////////////////////////

/*
 * Many places in the code want to bump tracing levels by some amount
 * for systemlib-related processing.  This is the amount they all bump
 * by.
 */
constexpr int kSystemLibBump = 10;

/*
 * Functions listed in the --trace functions list get trace level bumped by
 * this amount.
 */
constexpr int kTraceFuncBump = -10;

/*
 * We may run the interpreter collecting stats and when trace is on
 * the amount of noise is unbearable. This is to keep tracing out
 * of stats collection.
 */
constexpr int kStatsBump = 50;

//////////////////////////////////////////////////////////////////////

// "Bucketize" a vector of strings into N buckets, using consistent
// hashing.
std::vector<std::vector<SString>>
consistently_bucketize(const std::vector<SString>&, size_t bucketSize);

//////////////////////////////////////////////////////////////////////

// Helper functions to produce a std::vector<T> from a single T
// without boilerplate.
template <typename T> std::vector<T> singleton_vec(T t) {
  std::vector<T> out;
  out.emplace_back(std::move(t));
  return out;
}

//////////////////////////////////////////////////////////////////////

void profile_memory(const char* what, const char* when, const std::string&);
void summarize_memory(StructuredLogEntry*);

struct trace_time {
  explicit trace_time(const char* what,
                      std::string extra,
                      StructuredLogEntry* logEntry);
  explicit trace_time(const char* what)
    : trace_time{what, std::string{}, nullptr} {}
  explicit trace_time(const char* what, StructuredLogEntry* logEntry)
    : trace_time{what, std::string{}, logEntry} {}
  explicit trace_time(const char* what, std::string extra)
    : trace_time{what, std::move(extra), nullptr} {}
  ~trace_time();

  trace_time(const trace_time&) = delete;
  trace_time& operator=(const trace_time&) = delete;

  void ignore_client_stats();

  // Register a Client::Stats to automatically report on
  static void register_client_stats(extern_worker::Client::Stats::Ptr);

private:
  using clock      = std::chrono::system_clock;
  using time_point = clock::time_point;

  const char* what;
  time_point start;
  std::string extra;
  extern_worker::Client::Stats::Ptr clientBefore;
  StructuredLogEntry* logEntry;
  int64_t beforeRss;
};

//////////////////////////////////////////////////////////////////////

// Set up the process state (either for the main process or for
// extern-worker Jobs). If "full" is true, systemlib will be parsed
// and initialized.
void process_init(const Options&, const RepoGlobalData&, bool full);
// Undo process_init().
void process_exit();

}}

//////////////////////////////////////////////////////////////////////

namespace std {

//////////////////////////////////////////////////////////////////////

template<>
struct hash<HPHP::HHBBC::PrepKind> {
  size_t operator()(HPHP::HHBBC::PrepKind k) const {
    return k.hash();
  }
};

//////////////////////////////////////////////////////////////////////

}
