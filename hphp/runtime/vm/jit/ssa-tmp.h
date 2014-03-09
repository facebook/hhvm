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

#ifndef incl_HPHP_VM_SSATMP_H_
#define incl_HPHP_VM_SSATMP_H_

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace JIT {

class IRInstruction;
class IRUnit;
class IRBuilder;

class SSATmp {
public:
  uint32_t          id() const { return m_id; }
  IRInstruction*    inst() const { return m_inst; }
  void              setInstruction(IRInstruction* i, int dstId = 0);
  Type              type() const { return m_type; }
  void              setType(Type t) { m_type = t; }
  std::string       toString() const;

  // Convenience wrapper for type().isConst(...). See type.h for details.
  template<typename... Args>
  bool isConst(Args&&... args) const {
    return type().isConst(std::forward<Args>(args)...);
  }

  /*
   * For SSATmps with a compile-time constant value, the following
   * functions allow accessing it.
   *
   * Pre: inst() && isConst()
   */
  bool               boolVal() const      { return type().boolVal(); }
  int64_t            intVal() const       { return type().intVal(); }
  uint64_t           rawVal() const       { return type().rawVal(); }
  double             dblVal() const       { return type().dblVal(); }
  const StringData*  strVal() const       { return type().strVal(); }
  const ArrayData*   arrVal() const       { return type().arrVal(); }
  const Func*        funcVal() const      { return type().funcVal(); }
  const Class*       clsVal() const       { return type().clsVal(); }
  RDS::Handle        rdsHandleVal() const { return type().rdsHandleVal(); }
  TCA                tcaVal() const       { return type().tcaVal(); }
  Variant            variantVal() const;

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
  friend class IRBuilder;

  // May only be created via IRUnit.  Note that this class is never
  // destructed, so don't add complex members.
  SSATmp(uint32_t opndId, IRInstruction* inst, int dstId = 0);
  SSATmp(const SSATmp&) = delete;
  SSATmp& operator=(const SSATmp&) = delete;

  IRInstruction*  m_inst;
  Type            m_type; // type when defined
  const uint32_t  m_id;
};

}}

#endif
