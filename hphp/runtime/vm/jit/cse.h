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

#ifndef incl_HPHP_HHIR_CSE_H_
#define incl_HPHP_HHIR_CSE_H_

#include <unordered_map>

#include "folly/Hash.h"

#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP { namespace JIT {

/*
 * Hashtable used for common subexpression elimination.  The table maps
 * keys (instructions) to values (ssatmps).  Instructions are compared
 * by value (opcode and operands).
 */
struct CSEHash {
  SSATmp* lookup(IRInstruction* inst) {
    MapType::iterator it = map.find(inst);
    if (it == map.end()) {
      return nullptr;
    }
    return (*it).second;
  }
  SSATmp* insert(SSATmp* opnd) {
    return insert(opnd, opnd->inst());
  }
  SSATmp* insert(SSATmp* opnd, IRInstruction* inst) {
    return map[inst] = opnd;
  }
  void erase(SSATmp* opnd) {
    map.erase(opnd->inst());
  }
  void clear() {
    map.clear();
  }

  template<class... Args>
  static size_t instHash(Args&&... args) {
    return folly::hash::hash_combine(std::forward<Args>(args)...);
  }

  template<class... Args>
  static size_t hashCombine(size_t base, Args&&... args) {
    return folly::hash::hash_128_to_64(base,
      instHash(std::forward<Args>(args)...));
  }

private:
  struct EqualsOp {
    bool operator()(IRInstruction* i1, IRInstruction* i2) const {
      return i1->cseEquals(i2);
    }
  };

  struct HashOp {
    size_t operator()(IRInstruction* inst) const {
      return inst->cseHash();
    }
    size_t hash(IRInstruction* inst) const {
      return inst->cseHash();
    }
  };

  typedef std::unordered_map<IRInstruction*, SSATmp*,
                             HashOp, EqualsOp>  MapType;
  MapType map;
};

}}

#endif

