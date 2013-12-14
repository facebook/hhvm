/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/base.h"

namespace HPHP {
namespace JIT {

/*
 * Core types.
 */
typedef unsigned char* TCA; // "Translation cache adddress."
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

typedef uint32_t               TransID;
typedef hphp_hash_set<TransID> TransIDSet;
typedef std::vector<TransID>   TransIDVec;

const TransID InvalidID = -1LL;

/**
 * The different kinds of translations that the JIT generates:
 *
 *   - Anchor   : a service request for retranslating
 *   - Prologue : function prologue
 *   - Interp   : a service to interpret at least one instruction
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

enum TransKind {
#define DO(KIND) Trans##KIND,
  TRANS_KINDS
#undef DO
};

}}

#endif
