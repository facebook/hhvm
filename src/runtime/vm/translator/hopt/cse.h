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

#ifndef _CSE_H_
#define _CSE_H_

#include <tr1/unordered_map>

namespace HPHP {
namespace VM {
namespace JIT {

class CSEHash {
public:
  virtual ~CSEHash() {}
  virtual SSATmp* lookup(IRInstruction* inst) {
    MapType::iterator it = map.find(inst);
    if (it == map.end()) {
      return NULL;
    }
    return (*it).second;
  }
  void insert(SSATmp* opnd) {
    insert(opnd, opnd->getInstruction());
  }
  void insert(SSATmp* opnd, IRInstruction* inst) {
    map[inst] = opnd;
  }
  void clear() {
    map.clear();
  }

  // HHIR:TOOD switch to pointer_hash
  static inline uintptr_t ptrHash(void* ptr) {
    return ((uintptr_t)(ptr) >> 5);
  }
  static inline
  uint32 instHash(uint16 op, uint8 type) {
    return ((uint32)op << 8) + (uint32)type;
  }
  static inline
  uint32 instHash(uint16 op, uint8 type, void* src) {
    return instHash(op, type) ^ ptrHash(src);
  }
  static inline
  uint32 instHash(uint16 op, uint8 type, void* src1, void* src2) {
    return instHash(op, type, src1) ^ ptrHash(src2);
  }
  static inline
  uint32 instHash(uint16 op, uint8 type,
                  void* src1, void* src2, void* src3) {
    return instHash(op, type, src1, src2) ^ ptrHash(src3);
  }
  static inline
  uint32 instHash(uint16 op, uint8 type, int64 val) {
    return instHash(op, type) ^ val;
  }
  static inline
  uint32 instHash(uint16 op, uint8 type, void* src1, void* src2, int64 val) {
    return instHash(op, type, src1, src2) ^ val;
  }
private:
  struct EqualsOp {
    bool operator()(IRInstruction* i1, IRInstruction* i2) const {
      return i1->equals(i2);
    }
  };

  struct HashOp {
    size_t operator()(IRInstruction* inst) const {
      return inst->hash();
    }
    size_t hash(IRInstruction* inst) const {
      return inst->hash();
    }
  };
protected:
  typedef std::tr1::unordered_map<IRInstruction*, SSATmp*,
                                  HashOp, EqualsOp>  MapType;
  MapType map;

};

class ScopedCSEHash : public CSEHash {
public:
  ScopedCSEHash(CSEHash* p) : parent(p) {}
  virtual SSATmp* lookup(IRInstruction* inst) {
    SSATmp* opnd = map[inst];
    if (opnd) {
      return opnd;
    }
    if (parent) {
      return parent->lookup(inst);
    }
    return NULL;
  }
private:
  CSEHash* parent;
};

}}} // namespace HPHP::VM::JIT

#endif // _CSE_H_

