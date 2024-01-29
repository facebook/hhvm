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

#include <vector>

#include "hphp/runtime/base/types.h"

#include "hphp/runtime/vm/jit/containers.h"

#include "hphp/util/assertions.h"
#include "hphp/util/compact-tagged-ptrs.h"
#include "hphp/util/optional.h"
#include "hphp/util/hash-set.h"

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Core types.
 */
using TCA = unsigned char*; // "Translation cache address."
using CTCA = const unsigned char*;

using TcaRange = folly::Range<TCA>;

using LowTCA = LowPtr<uint8_t>;
using AtomicLowTCA = AtomicLowPtr<uint8_t,
                                  std::memory_order_acquire,
                                  std::memory_order_release>;

struct ctca_identity_hash {
  size_t operator()(CTCA val) const {
    // Experiments show that this is a sufficient "hash function" on
    // TCAs for now; using stronger functions didn't help given current
    // data. Patterns of code emission in the translator could invalidate
    // this finding going forward, though; e.g., if we frequently emit
    // a call instruction N bytes into a cache-aligned region.
    return uintptr_t(val);
  }
};

///////////////////////////////////////////////////////////////////////////////

using TransIDSet = jit::flat_set<TransID>;
using TransIDVec = std::vector<TransID>;

using Annotation = std::pair<std::string, std::string>;
using Annotations = std::vector<Annotation>;

///////////////////////////////////////////////////////////////////////////////

/**
 * The different kinds of translations that the JIT generates:
 *
 *   - Anchor       : a service request for retranslating
 *   - Interp       : a service request to interpret at least one instruction
 *   - Live         : translate one tracelet by inspecting live VM state
 *   - Profile      : translate one block by inspecting live VM state and
 *                    inserting profiling counters
 *   - Optimize     : translate one region performing optimizations that may
 *                    leverage data collected by Profile translations
 *   - LivePrologue : prologue for a function being JITed in Live mode
 *   - ProfPrologue : prologue for a function being JITed in Profile mode
 *   - OptPrologue  : prologue for a function being JITed in Optimize mode
 */
#define TRANS_KINDS \
    DO(Anchor)      \
    DO(Interp)      \
    DO(Live)        \
    DO(Profile)     \
    DO(Optimize)    \
    DO(LivePrologue)\
    DO(ProfPrologue)\
    DO(OptPrologue) \
    DO(Invalid)     \

enum class TransKind {
#define DO(KIND) KIND,
  TRANS_KINDS
#undef DO
};

constexpr size_t NumTransKinds =
#define DO(KIND) + 1
  TRANS_KINDS
#undef DO
  ;

inline std::string show(TransKind k) {
#define DO(name) case TransKind::name: return "Trans" #name;
  switch (k) { TRANS_KINDS }
#undef DO
  not_reached();
}

inline Optional<TransKind> nameToTransKind(const std::string& str) {
#define DO(name) if (str == "Trans" #name) return TransKind::name;
  TRANS_KINDS
#undef DO
  return std::nullopt;
}

inline bool isProfiling(TransKind k) {
  switch (k) {
    case TransKind::Profile:
    case TransKind::ProfPrologue:
      return true;

    case TransKind::Anchor:
    case TransKind::Interp:
    case TransKind::Live:
    case TransKind::LivePrologue:
    case TransKind::Optimize:
    case TransKind::OptPrologue:
    case TransKind::Invalid:
      return false;
  }
  always_assert(false);
}

inline bool isPrologue(TransKind k) {
  switch (k) {
    case TransKind::LivePrologue:
    case TransKind::ProfPrologue:
    case TransKind::OptPrologue:
      return true;

    case TransKind::Anchor:
    case TransKind::Interp:
    case TransKind::Live:
    case TransKind::Profile:
    case TransKind::Optimize:
    case TransKind::Invalid:
      return false;
  }
  always_assert(false);
}

inline bool isOptimized(TransKind k) {
  switch (k) {
    case TransKind::Optimize:
    case TransKind::OptPrologue:
      return true;

    case TransKind::Profile:
    case TransKind::ProfPrologue:
    case TransKind::Anchor:
    case TransKind::Interp:
    case TransKind::Live:
    case TransKind::LivePrologue:
    case TransKind::Invalid:
      return false;
  }
  always_assert(false);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * The "kind" of code being generated.
 *
 * Different contexts of code generation constrain codegen differently; e.g.,
 * cross-trace code has fewer available registers.
 */
enum class CodeKind : uint8_t {
  /*
   * Normal PHP code in the TC.
   */
  Trace,

  /*
   * Code for function prologues. Similar to CrossTrace, but may allow more
   * registers.
   */
  Prologue,

  /*
   * Code at the TC boundaries, e.g., service requests, unique stubs.
   */
  CrossTrace,

  /*
   * Helper code that uses native scratch registers only.
   *
   * This roughly means unreserved, caller-saved, non-argument registers---but
   * best to just look at the helper ABI in the appropriate abi-*.cpp file.
   */
  Helper,
};

inline std::string codeKindAsString(CodeKind kind) {
  switch (kind) {
    case CodeKind::Trace:      return "trace";
    case CodeKind::Prologue:   return "prologue";
    case CodeKind::CrossTrace: return "cross-trace";
    case CodeKind::Helper:     return "helper";
  }
  always_assert(false);
}

/*
 * Enumeration representing the various areas that we emit code.
 *
 * kNumAreas must be kept up to date.
 */
enum class AreaIndex : uint8_t { Main, Cold, Frozen };
constexpr size_t kNumAreas = 3;

inline std::string areaAsString(AreaIndex area) {
  switch (area) {
  case AreaIndex::Main:
    return "Main";
  case AreaIndex::Cold:
    return "Cold";
  case AreaIndex::Frozen:
    return "Frozen";
  }
  always_assert(false);
}

inline Optional<AreaIndex> nameToAreaIndex(const std::string name) {
  if (name == "Main") return AreaIndex::Main;
  if (name == "Cold") return AreaIndex::Cold;
  if (name == "Frozen") return AreaIndex::Frozen;
  return std::nullopt;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Encapsulates a TCA for a translation, or a "scope" for the failure
 * to produce one. The scope is a bound for how long the failure is
 * expected to persistent.
 */
struct TranslationResult {
  enum class Scope : uint16_t {
    Success,   // Have a TCA
    Transient, // Transient failure. The next attempt might succeed.
    Request,   // Attempts will fail until the next request at least.
    Process    // Attempts will fail for the remainder of the process
               // lifetime.
  };

  explicit TranslationResult(TCA tca)
    : m_ptr{Scope::Success, tca} { assertx(tca); }
  explicit TranslationResult(Scope s)
    : m_ptr{s, nullptr} { assertx(s != Scope::Success); }

  static TranslationResult failTransiently() {
    return TranslationResult{Scope::Transient};
  }
  static TranslationResult failForProcess() {
    return TranslationResult{Scope::Process};
  }

  TCA addr() const { return m_ptr.ptr(); }
  Scope scope() const { return m_ptr.tag(); }

  bool isRequestPersistentFailure() const {
    auto const s = scope();
    return s == Scope::Request || s == Scope::Process;
  }
  bool isProcessPersistentFailure() const {
    return scope() == Scope::Process;
  }

private:
  CompactTaggedPtr<unsigned char, Scope> m_ptr;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Some data structures are accessed often enough from translated code that we
 * have shortcuts for getting offsets into them.
 */
#define TVOFF(nm) int(offsetof(TypedValue, nm))
#define AROFF(nm) int(offsetof(ActRec, nm))
#define AFWHOFF(nm) int(offsetof(c_AsyncFunctionWaitHandle, nm))
#define GENDATAOFF(nm) int(offsetof(Generator, nm))

///////////////////////////////////////////////////////////////////////////////

/*
 * Generalization of Status Flag bits encoded in Vinstr and used by the
 * annotateSFUses() pass and platform-specific lowerers/emitters.
 *
 * In order for a platform to utilize the pass, they'll need to implement
 * mappings between ConditionCodes and an operator|-able bit sequence held in a
 * Vflags byte.  This implies that the platform will need to define their
 * status flag bits as well.  See required_flags() in abi-arm.h for an example.
 */
using Vflags = uint8_t;

///////////////////////////////////////////////////////////////////////////////

/*
 * Information attached to an assertion in emitted code.
 */
struct Reason {
  const char* file;
  unsigned line;

  bool operator==(const Reason& o) const {
    return line == o.line && std::string{file} == std::string{o.file};
  }
  bool operator!=(const Reason& o) const {
    return line != o.line || std::string{file} != std::string{o.file};
  }
};

inline std::string show(const Reason &r) {
  return folly::sformat("{}:{}", r.file, r.line);
}

enum class LdClsFallback: uint8_t {
  FATAL,
  FATAL_RESOLVE_CLASS,
  THROW_CLASSNAME_TO_CLASS_STRING,
  THROW_CLASSNAME_TO_CLASS_LAZYCLASS,
};

}
