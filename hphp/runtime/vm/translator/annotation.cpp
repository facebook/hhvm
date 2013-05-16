/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/util/base.h"
#include "hphp/runtime/vm/translator/translator.h"
#include "hphp/runtime/vm/translator/translator-inline.h"
#include "hphp/runtime/vm/translator/annotation.h"

namespace HPHP {
namespace Transl {

static const Trace::Module TRACEMOD = Trace::trans;

/*
 * A mapping from FCall instructions to the statically-known StringData*
 * that they're calling. Used to accelerate our FCall translations.
 */
enum CallRecordType {
  EncodedNameAndArgs,
  Function
};
struct CallRecord {
  CallRecordType m_type;
  union {
    const StringData* m_encodedName;
    const Func* m_func;
  };
};

typedef hphp_hash_map<SrcKey, CallRecord, SrcKey> CallDB;
static CallDB s_callDB;
/* record the max number of args to enable invalidation */
static int s_maxNumArgs;
int getMaxEncodedArgs() { return s_maxNumArgs; }

const StringData*
encodeCallAndArgs(const StringData* name, int numArgs) {
  char numArgsBuf[16];
  if (numArgs > s_maxNumArgs) s_maxNumArgs = numArgs;
  snprintf(numArgsBuf, 15, "@%d@", numArgs);
  String s = String(numArgsBuf) + String(name->data());
  return StringData::GetStaticString(s.get());
}

static void
decodeNameAndArgs(const StringData* enc, string& outName, int& outNumArgs) {
  const char* numArgs = strchr(enc->data(), '@');
  assert(numArgs && *numArgs =='@');
  numArgs++;
  outNumArgs = atoi(numArgs);
  const char* name = strchr(numArgs, '@');
  assert(name && *name == '@');
  name++;
  outName = name;
}

static void recordNameAndArgs(const SrcKey& sk,
                              const StringData* name,
                              int numArgs) {
  CallRecord cr;
  cr.m_type = EncodedNameAndArgs;
  cr.m_encodedName = encodeCallAndArgs(name, numArgs);
  s_callDB.insert(std::make_pair(sk, cr));
}

static void recordFunc(NormalizedInstruction& i,
                       const SrcKey& sk,
                       const Func* func) {
  FTRACE(2, "annotation: recordFunc: {}@{} {}\n",
         i.m_unit->filepath()->data(),
         sk.offset(),
         func->fullName()->data());

  CallRecord cr;
  cr.m_type = Function;
  cr.m_func = func;
  s_callDB.insert(std::make_pair(sk, cr));
  i.directCall = true;
}

static void recordActRecPush(NormalizedInstruction& i,
                             const Unit* unit,
                             const StringData* name,
                             const StringData* clsName,
                             bool staticCall) {
  const SrcKey& sk = i.source;
  FTRACE(2, "annotation: recordActRecPush: {}@{} {}{}{} ({}static)\n",
         unit->filepath()->data(),
         sk.offset(),
         clsName ? clsName->data() : "",
         clsName ? "::" : "",
         name,
         !staticCall ? "non" : "");

  SrcKey next(sk);
  next.advance(unit);
  const FPIEnt *fpi = curFunc()->findFPI(next.offset());
  assert(fpi);
  assert(name->isStatic());
  assert(sk.offset() == fpi->m_fpushOff);
  SrcKey fcall = sk;
  fcall.m_offset = fpi->m_fcallOff;
  assert(isFCallStar(*unit->at(fcall.offset())));
  if (clsName) {
    const Class* cls = Unit::lookupUniqueClass(clsName);
    bool magic = false;
    const Func* func = lookupImmutableMethod(cls, name, magic, staticCall);
    if (func) {
      recordFunc(i, fcall, func);
    }
    return;
  }
  const Func* func = Unit::lookupFunc(name);
  if (func && func->isNameBindingImmutable(unit)) {
    // this will never go into a call cache, so we dont need to
    // encode the args. it will be used in OpFCall below to
    // set the i->funcd.
    recordFunc(i, fcall, func);
  } else {
    // It's not enough to remember the function name; we also need to encode
    // the number of arguments and current flag disposition.
    int numArgs = getImm(unit->at(sk.offset()), 0).u_IVA;
    recordNameAndArgs(fcall, name, numArgs);
  }
}

void annotate(NormalizedInstruction* i) {
  switch(i->op()) {
    case OpFPushObjMethodD:
    case OpFPushClsMethodD:
    case OpFPushClsMethodF:
    case OpFPushCtorD:
    case OpFPushCtor:
    case OpFPushFuncD: {
      // When we push predictable action records, we can use a simpler
      // translation for their corresponding FCall.
      const StringData* className = nullptr;
      const StringData* funcName = nullptr;
      if (i->op() == OpFPushFuncD) {
        funcName = curUnit()->lookupLitstrId(i->imm[1].u_SA);
      } else if (i->op() == OpFPushObjMethodD) {
        if (i->inputs[0]->valueType() != KindOfObject) break;
        const Class* cls = i->inputs[0]->rtt.valueClass();
        if (!cls) break;
        funcName = curUnit()->lookupLitstrId(i->imm[1].u_SA);
        className = cls->name();
      } else if (i->op() == OpFPushClsMethodF) {
        if (!i->inputs[1]->isString() ||
            i->inputs[1]->rtt.valueString() == nullptr ||
            i->inputs[0]->valueType() != KindOfClass) {
          break;
        }
        const Class* cls = i->inputs[0]->rtt.valueClass();
        if (!cls) break;
        funcName = i->inputs[1]->rtt.valueString();
        className = cls->name();
      } else if (i->op() == OpFPushClsMethodD) {
        funcName = curUnit()->lookupLitstrId(i->imm[1].u_SA);
        className = curUnit()->lookupLitstrId(i->imm[2].u_SA);
      } else if (i->op() == OpFPushCtorD) {
        className = curUnit()->lookupLitstrId(i->imm[1].u_SA);
        const Class* cls = Unit::lookupUniqueClass(className);
        if (!cls) break;
        funcName = cls->getCtor()->name();
      } else {
        assert(i->op() == OpFPushCtor);
        const Class* cls = i->inputs[0]->rtt.valueClass();
        if (!cls) break;
        funcName = cls->getCtor()->name();
      }
      assert(funcName->isStatic());
      recordActRecPush(*i, curUnit(), funcName, className,
                       i->op() == OpFPushClsMethodD ||
                       i->op() == OpFPushClsMethodF);
    } break;
    case OpFCall:
    case OpFCallArray: {
      CallRecord callRec;
      if (mapGet(s_callDB, i->source, &callRec)) {
        if (callRec.m_type == Function) {
          i->funcd = callRec.m_func;
        } else {
          assert(callRec.m_type == EncodedNameAndArgs);
          i->funcName = callRec.m_encodedName;
        }
      } else {
        i->funcName = nullptr;
      }
    } break;
    default: break;
  }
}

const StringData*
fcallToFuncName(const NormalizedInstruction* i) {
  CallRecord callRec;
  if (mapGet(s_callDB, i->source, &callRec)) {
    if (callRec.m_type == Function) {
      return callRec.m_func->name();
    }
    string name;
    int numArgs;
    decodeNameAndArgs(callRec.m_encodedName, name, numArgs);
    return StringData::GetStaticString(name.c_str());
  }
  return nullptr;
}

} }

