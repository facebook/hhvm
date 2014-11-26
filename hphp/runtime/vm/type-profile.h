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

#ifndef TYPE_PROFILE_H_
#define TYPE_PROFILE_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/hhbc.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct StringData;
struct Func;

//////////////////////////////////////////////////////////////////////

struct TypeProfileKey {
  enum KeyType {
    MethodName,
    PropName,
    EltName,
    StaticPropName
  } m_kind;
  const StringData* m_name;

  TypeProfileKey(KeyType kind, const StringData* sd) :
   m_kind(kind), m_name(sd) { }

  TypeProfileKey(MemberCode mc, const StringData* sd) :
    m_kind(mc == MET ? EltName : PropName), m_name(sd) { }
  uint64_t hash() const;
};

// These are both best-effort, and return noisy results.
void profileInit();
void profileWarmupStart();
void profileWarmupEnd();
void profileRequestStart();
void profileRequestEnd();
void recordType(TypeProfileKey sk, DataType dt);
std::pair<MaybeDataType, double> predictType(TypeProfileKey key);
int64_t requestCount();

/*
 * Profiling for func hotness goes through this module.
 */
void profileIncrementFuncCounter(const Func*);

extern __thread bool profileOn;
inline bool isProfileRequest() {
  return profileOn;
}

extern __thread bool standardRequest;
inline bool isStandardRequest() {
  return standardRequest;
}

void setRelocateRequests(int32_t n);
//////////////////////////////////////////////////////////////////////

}

#endif
