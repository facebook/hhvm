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

#include "hphp/runtime/vm/jit/extra-data.h"

#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/util/text-util.h"

namespace HPHP { namespace jit {

std::string NewStructData::show() const {
  std::ostringstream os;
  os << offset.offset << ',';
  auto delim = "";
  for (uint32_t i = 0; i < numKeys; i++) {
    os << delim << "\"" <<
       escapeStringForCPP(keys[i]->data(), keys[i]->size()) <<
       "\"";
    delim = ",";
  }
  return os.str();
}

//////////////////////////////////////////////////////////////////////

namespace {

FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_hash,   hash);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_equals, equals);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_clone,  clone);
FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_show,   show);

/*
 * dispatchExtra translates from runtime values for the Opcode enum
 * into compile time types.  The goal is to call a `targetFunction'
 * that is overloaded on the extra data type structs.
 *
 * The purpose of the MAKE_DISPATCHER layer is to weed out Opcode
 * values that have no associated extra data.
 *
 * Basically this is doing dynamic dispatch without a vtable in
 * IRExtraData, instead using the Opcode tag from the associated
 * instruction to discriminate the runtime type.
 *
 * Note: functions made with this currently only make sense to call if
 * it's already known that the opcode has extra data.  If you call it
 * for one that doesn't, you'll get an abort.  Generally hasExtra()
 * should be checked first.
 */

#define MAKE_DISPATCHER(name, rettype, targetFunction)                \
  template<bool HasExtra, Opcode opc> struct name {                   \
    template<class... Args>                                           \
    static rettype go(IRExtraData* vp, Args&&...) { not_reached(); }  \
  };                                                                  \
  template<Opcode opc> struct name<true,opc> {                        \
    template<class... Args>                                           \
    static rettype go(IRExtraData* vp, Args&&... args) {              \
      return targetFunction(                                          \
        static_cast<typename IRExtraDataType<opc>::type*>(vp),        \
        std::forward<Args>(args)...                                   \
      );                                                              \
    }                                                                 \
  };

template<
  class RetType,
  template<bool, Opcode> class Dispatcher,
  class... Args
>
RetType dispatchExtra(Opcode opc, IRExtraData* data, Args&&... args) {
#define O(opcode, dstinfo, srcinfo, flags)      \
  case opcode:                                  \
    return Dispatcher<                          \
      OpHasExtraData<opcode>::value,            \
      opcode                                    \
    >::go(data, std::forward<Args>(args)...);
  switch (opc) { IR_OPCODES default: not_reached(); }
#undef O
  not_reached();
}

template<class T>
typename std::enable_if<
  has_hash<T,size_t () const>::value,
  size_t
>::type hashExtraImpl(T* t) { return t->hash(); }
size_t hashExtraImpl(IRExtraData*) {
  // This probably means we tried to hash an IRInstruction with extra data that
  // had no hash function.
  always_assert(!"attempted to hash extra data that didn't "
    "provide a hash function");
}

template<class T>
typename std::enable_if<
  has_equals<T,bool (T const&) const>::value ||
  has_equals<T,bool (T)        const>::value,
  bool
>::type equalsExtraImpl(T* t, IRExtraData* o) {
  return t->equals(*static_cast<T*>(o));
}
bool equalsExtraImpl(IRExtraData*, IRExtraData*) {
  // This probably means we tried to compare IRInstructions with extra data that
  // had no equals function.
  always_assert(!"attempted to compare extra data that didn't "
                 "provide an equals function");
}

// Clone using a data-specific clone function.
template<class T>
typename std::enable_if<
  has_clone<T,T* (Arena&) const>::value,
  T*
>::type cloneExtraImpl(T* t, Arena& arena) {
  return t->clone(arena);
}

// Use the copy constructor if no clone() function was supplied.
template<class T>
typename std::enable_if<
  !has_clone<T,T* (Arena&) const>::value,
  T*
>::type cloneExtraImpl(T* t, Arena& arena) {
  return new (arena) T(*t);
}

template<class T>
std::string showExtraImpl(const T* extra) { return extra->show(); }

MAKE_DISPATCHER(HashDispatcher, size_t, hashExtraImpl);
MAKE_DISPATCHER(EqualsDispatcher, bool, equalsExtraImpl);
MAKE_DISPATCHER(CloneDispatcher, IRExtraData*, cloneExtraImpl);
MAKE_DISPATCHER(ShowDispatcher, std::string, showExtraImpl);

}

//////////////////////////////////////////////////////////////////////

size_t hashExtra(Opcode opc, const IRExtraData* data) {
  return dispatchExtra<size_t,HashDispatcher>(
    opc, const_cast<IRExtraData*>(data));
}

bool equalsExtra(
  Opcode opc,
  const IRExtraData* data,
  const IRExtraData* other
) {
  return dispatchExtra<bool,EqualsDispatcher>(
    opc, const_cast<IRExtraData*>(data), const_cast<IRExtraData*>(other));
}

IRExtraData* cloneExtra(Opcode opc, IRExtraData* data, Arena& a) {
  return dispatchExtra<IRExtraData*,CloneDispatcher>(opc, data, a);
}

std::string showExtra(Opcode opc, const IRExtraData* data) {
  return dispatchExtra<std::string,ShowDispatcher>(
    opc, const_cast<IRExtraData*>(data)
  );
}

//////////////////////////////////////////////////////////////////////

}}
