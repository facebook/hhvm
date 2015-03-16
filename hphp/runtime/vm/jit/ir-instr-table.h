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

#ifndef incl_HPHP_IR_INSTR_TABLE_H_
#define incl_HPHP_IR_INSTR_TABLE_H_

#include <unordered_map>

#include <folly/Hash.h>

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Hash table used for IRUnit constants.
 *
 * The table maps IRInstructions to SSATmps.  Instructions are considered to
 * be the same iff their inputs and parameters are the same.
 */
struct IRInstrTable {
  SSATmp* lookup(IRInstruction* inst) const {
    auto const it = m_map.find(inst);
    if (it == m_map.end()) return nullptr;
    return it->second;
  }
  SSATmp* insert(SSATmp* opnd) {
    return m_map[opnd->inst()] = opnd;
  }
  void erase(SSATmp* opnd) {
    m_map.erase(opnd->inst());
  }
  void clear() {
    m_map.clear();
  }

private:
  struct EqualsOp {
    bool operator()(const IRInstruction* i1,
                    const IRInstruction* i2) const {
      if (i1->op()           != i2->op() ||
          i1->hasTypeParam() != i2->hasTypeParam() ||
          i1->numSrcs()      != i2->numSrcs() ||
          i1->hasExtra()     != i2->hasExtra()) {
        return false;
      }
      if (i1->hasTypeParam()) {
        if (i1->typeParam() != i2->typeParam()) return false;
      }
      for (uint32_t i = 0; i < i1->numSrcs(); ++i) {
        if (i1->src(i) != i2->src(i)) {
          return false;
        }
      }
      if (i1->hasExtra()) {
        return equalsExtra(i1->op(), i1->rawExtra(), i2->rawExtra());
      }
      return true;
    }
  };

  struct HashOp {
    size_t hash(const IRInstruction* inst) const {
      size_t h = 0;
      for (uint32_t i = 0; i < inst->numSrcs(); ++i) {
        h = hash_combine(h, inst->src(i));
      }
      if (inst->hasExtra()) {
        h = hash_combine(h, hashExtra(inst->op(), inst->rawExtra()));
      }
      if (inst->hasTypeParam()) {
        h = hash_combine(h, inst->typeParam());
      }
      return hash_combine(h, inst->op());
    }
    size_t operator()(IRInstruction* inst) const {
      return hash(inst);
    }

  private:
    template<class T>
    static size_t hash_combine(size_t base, T other) {
      return folly::hash::hash_128_to_64(
          base, folly::hash::hash_combine(other));
    }
  };

private:
  std::unordered_map<IRInstruction*,SSATmp*,HashOp,EqualsOp> m_map;
};

///////////////////////////////////////////////////////////////////////////////
}}

#endif
