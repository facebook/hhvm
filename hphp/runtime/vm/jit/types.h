/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TRANSL_TYPES_H_
#define incl_HPHP_TRANSL_TYPES_H_

#include <vector>

#include "hphp/util/assertions.h"
#include "hphp/util/hash-map-typedefs.h"

#include "hphp/runtime/base/types.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Core types.
 */
typedef unsigned char* TCA; // "Translation cache address."
typedef const unsigned char* CTCA;

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

typedef hphp_hash_set<TransID> TransIDSet;
typedef std::vector<TransID>   TransIDVec;

///////////////////////////////////////////////////////////////////////////////

/**
 * The different kinds of translations that the JIT generates:
 *
 *   - Anchor   : a service request for retranslating
 *   - Prologue : function prologue
 *   - Interp   : a service request to interpret at least one instruction
 *   - Live     : translate one tracelet by inspecting live VM state
 *   - Profile  : translate one block by inspecting live VM state and
 *                inserting profiling counters
 *   - Optimize : translate one region performing optimizations that may
 *                leverage data collected by Profile translations
 *   - Proflogue: a profiling function prologue
 */
#define TRANS_KINDS \
    DO(Anchor)      \
    DO(Prologue)    \
    DO(Interp)      \
    DO(Live)        \
    DO(Profile)     \
    DO(Optimize)    \
    DO(Proflogue)   \
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

/*
 * Compact flags which may be threaded through a service request to provide
 * hints or demands for retranslations.
 */
struct TransFlags {
  /* implicit */ TransFlags(uint64_t flags = 0) : packed(flags) {}

  union {
    struct {
      bool noinlineSingleton : 1;
      bool noProfiledFPush : 1;
    };
    uint64_t packed;
  };
};

static_assert(sizeof(TransFlags) <= sizeof(uint64_t), "Too many TransFlags!");

///////////////////////////////////////////////////////////////////////////////

/*
 * The "kind" of code being generated.
 *
 * Different contexts of code generation constrain codegen differently; e.g.,
 * cross-trace code has fewer available registers.
 */
enum class CodeKind {
  Trace,
  CrossTrace,
};

/*
 * Enumeration representing the various areas that we emit code.
 *
 * kNumAreas must be kept up to date.
 */
enum class AreaIndex : unsigned { Main, Cold, Frozen };
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

///////////////////////////////////////////////////////////////////////////////

}}

#endif
