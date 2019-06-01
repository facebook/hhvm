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

#ifndef incl_HPHP_JIT_CALL_TARGET_PROFILE_H_
#define incl_HPHP_JIT_CALL_TARGET_PROFILE_H_

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/frequency-profile.h"

namespace HPHP {

struct Class;

namespace jit {

///////////////////////////////////////////////////////////////////////////////

struct ProfDataSerializer;
struct ProfDataDeserializer;

/*
 * Profiles the target functions for a given call site to determine the most
 * common callee along with the expected probability of calling it.
 */
struct CallTargetProfile {

  void report(const ActRec* ar);

  static void reduce(CallTargetProfile& profile,
                     const CallTargetProfile& other);

  const Func* choose(double& probability) const;

  std::string toString() const;

  void serialize(ProfDataSerializer&) const;

  void deserialize(ProfDataDeserializer&);

private:
  FrequencyProfile<FuncId, 3, InvalidFuncId> m_profile;
};

///////////////////////////////////////////////////////////////////////////////

inline const StringData* callTargetProfileKey() {
  return makeStaticString("CallTargetProfile");
}

///////////////////////////////////////////////////////////////////////////////


}}

#endif
