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

#ifndef incl_HPHP_VM_SSATMP_H_
#define incl_HPHP_VM_SSATMP_H_

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace JIT {

class IRInstruction;
class IRUnit;
class TraceBuilder;

class SSATmp {
public:
  uint32_t          id() const { return m_id; }
  IRInstruction*    inst() const { return m_inst; }
  void              setInstruction(IRInstruction* i) { m_inst = i; }
  Type              type() const { return m_type; }
  void              setType(Type t) { m_type = t; }
  bool              isBoxed() const { return type().isBoxed(); }
  bool              isString() const { return isA(Type::Str); }
  bool              isArray() const { return isA(Type::Arr); }
  std::string       toString() const;

  // XXX: false for Null, etc.  Would rather it returns whether we
  // have a compile-time constant value.
  bool isConst() const;

  /*
   * For SSATmps with a compile-time constant value, the following
   * functions allow accessing it.
   *
   * Pre: inst() &&
   *   (inst()->op() == DefConst ||
   *    inst()->op() == LdConst)
   */
  bool               getValBool() const;
  int64_t            getValInt() const;
  int64_t            getValRawInt() const;
  double             getValDbl() const;
  const StringData*  getValStr() const;
  const ArrayData*   getValArr() const;
  const Func*        getValFunc() const;
  const Class*       getValClass() const;
  const NamedEntity* getValNamedEntity() const;
  RDS::Handle        getValRDSHandle() const;
  uintptr_t          getValBits() const;
  Variant            getValVariant() const;
  TCA                getValTCA() const;
  uintptr_t          getValCctx() const;

  /*
   * Returns: Type::subtypeOf(type(), tag).
   *
   * This should be used for most checks on the types of IRInstruction
   * sources.
   */
  bool isA(Type tag) const {
    return type().subtypeOf(tag);
  }

  /*
   * The maximum number of words this SSATmp may need allocated.
   * This is based on the type of the temporary (some types never have
   * regs, some have two, etc).
   */
  int numWords() const;

private:
  friend class IRUnit;
  friend class TraceBuilder;

  // May only be created via IRUnit.  Note that this class is never
  // destructed, so don't add complex members.
  SSATmp(uint32_t opndId, IRInstruction* i, int dstId = 0);
  SSATmp(const SSATmp&) = delete;
  SSATmp& operator=(const SSATmp&) = delete;

  IRInstruction*  m_inst;
  Type            m_type; // type when defined
  const uint32_t  m_id;
};

}}

#endif
