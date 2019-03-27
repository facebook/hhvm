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

#ifndef TYPE_PROFILE_H_
#define TYPE_PROFILE_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/rds-local.h"
#include "hphp/runtime/vm/hhbc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Func;

enum class RequestKind {
  Warmup,
  Profile,
  NonVM,
  Standard,
};

//////////////////////////////////////////////////////////////////////

/*
 * Used to indicate that the current thread should be ignored for profiling
 * purposes, usually because it is a JIT worker thread and not processing real
 * requests.
 */
struct ProfileNonVMThread {
  ProfileNonVMThread();
  ~ProfileNonVMThread();
};

void profileWarmupStart();
void profileWarmupEnd();
void profileRequestStart();
void profileRequestEnd();
void profileSetHotFunc();

int64_t requestCount();

/*
 * Profiling for func hotness goes through this module.
 */
void profileIncrementFuncCounter(const Func*);

struct TypeProfileLocals {
  RequestKind requestKind = RequestKind::Warmup;
  bool forceInterpret = false;
  bool nonVMThread = false;
};

extern RDS_LOCAL_NO_CHECK(TypeProfileLocals, rl_typeProfileLocals);

inline bool isProfileRequest() {
  return rl_typeProfileLocals->requestKind == RequestKind::Profile;
}

inline bool isStandardRequest() {
  return rl_typeProfileLocals->requestKind == RequestKind::Standard;
}

inline bool isForcedToInterpret() {
  return rl_typeProfileLocals->forceInterpret;
}

void setRelocateRequests(int32_t n);
//////////////////////////////////////////////////////////////////////

}

#endif
