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
#ifndef __HHIR_MEMELIM_H__
#define __HHIR_MEMELIM_H__


#include "ir.h"
#include "opt.h"
#include <runtime/base/util/countable.h>


namespace HPHP {
namespace VM {
namespace JIT {


void optimizeMemoryAccesses(Trace* trace, IRFactory* factory);


class MemMap {
 public:
  MemMap(IRFactory* factory) : factory(factory) {
    ASSERT(factory != NULL);
  }

  ~MemMap() {
    clearCountedMap(unescaped);
    clearCountedMap(unknown);
    clearCountedMap(locs);
    clearCountedMap(props);
  }

  // updates the memory map given the information from the new instruction
  void processInstruction(IRInstruction* inst);

  // returns the last memory access of a ref, or an object property. an offset
  // of -1 means that the ssa temp is a ref or a local, otherwise the temp is
  // an object and we're trying to access a property with that offset. if the
  // last access of the ref is not known, then this function returns NULL
  IRInstruction* getLastAccess(SSATmp* ref, int offset = -1);

  // returns the value stored in a memory location if it is still available.
  // the arguments are specified the same way as getLastAccess(). if the value
  // of the ref is not known, then this function returns NULL
  SSATmp* getValue(SSATmp* ref, int offset = -1);

  // removes redundant loads and dead stores, and sinks partially dead stores
  // into the exit edges on which they are live
  void optimizeMemoryAccesses(Trace* trace);

 private:
  // a class that will delete itself once its ref count reaches 0
  struct Counted : public Countable {
    // construct with a ref count of 1, Countable starts at 0
    Counted() { incRefCount(); }

    void inc() { incRefCount(); }

    int32_t dec() {
      if (decRefCount() == 0) {
        delete this;
        return 0;
      }

      return getCount();
    }
  };

  // known information about a box. this object tracks its last access as well
  // as its value. if the information represented by one of the fields is
  // unknown then it will be NULL. this object inherits from Counted such that
  // it can also track the reference count of the box for better availability
  // analysis. and when the object gets to a ref count of 0 it will be destroyed
  struct RefInfo : public Counted {
    RefInfo(IRInstruction* inst) {
      update(inst);
    }

    void update(IRInstruction* inst) {
      ASSERT(inst != NULL);
      access = inst;
      value = findValue(inst);
    }

    IRInstruction* access;
    SSATmp* value;
  };

  // known information about a property access. this object is like RefInfo,
  // except it also stores the offset of the property access and that it is not
  // ref counted
  struct PropInfo {
    PropInfo(IRInstruction* inst, int off)
      : access(inst), value(findValue(inst)), offset(off) {
      ASSERT(inst != NULL);
      ASSERT(off >= 0);
    }

    IRInstruction* access;
    SSATmp* value;
    int offset;
  };

  typedef std::list<PropInfo> PropList;

  // a list of information about an object's property accesses. PropInfoList
  // is ref counted to track the count of the object. each info object in the
  // list has a unique offset
  struct PropInfoList : public Counted {
    void update(IRInstruction* inst) {
      ASSERT(inst != NULL);

      int offset = inst->getSrc(1)->getConstValAsInt();

      PropList::iterator it, end;
      for (it = accesses.begin(), end = accesses.end(); it != end; ++it) {
        if (it->offset == offset) {
          it->access = inst;
          it->value = inst->getSrc(2);
        }
      }

      accesses.push_back(PropInfo(inst, offset));
    }

    PropList accesses;
  };

  // maps a box to a block of information about that box. boxes that are known
  // to alias will be mapped to the same object
  typedef hphp_hash_map<SSATmp*, RefInfo*> RefMap;

  // maps objects to a block of information about their property accesses
  typedef hphp_hash_map<SSATmp*, PropInfoList*> PropMap;
  typedef RefMap LocalsMap;

  // a mapping of stores to a vector of guarded instructions that follow them.
  // this is used to track where we need to sink partially dead stores.
  typedef std::list<std::pair<IRInstruction*, std::vector<IRInstruction*> > >
    StoreList;

  // kills information in our 'unknown' map
  void killRefInfo(IRInstruction* save);
  // kills information in our 'props' map
  void killPropInfo(IRInstruction* save);

  // escapes a box (and all of its aliases) from the 'unescaped' map into the
  // 'unknown' map
  void escapeRef(SSATmp* ref);

  void optimizeLoad(IRInstruction* inst, int offset = -1);
  void sinkStores(StoreList& stores);

  RefMap unescaped;   // refs that are known not to have escaped
  RefMap unknown;     // refs that have escaped or possibly escaped
  LocalsMap locs;     // locals
  PropMap props;      // property accesses

  IRFactory* factory; // needed to clone instructions for sinking

  template <typename V>
  static inline void clearCountedMap(hphp_hash_map<SSATmp*, V*>& map) {
    typename hphp_hash_map<SSATmp*, V*>::iterator it, copy, end;
    for (it = map.begin(), end = map.end(); it != end; ) {
      copy = it;
      ++it;
      if (copy->first->getInstruction()->getOpcode() != Mov) {
        copy->second->dec();
      }
      map.erase(copy);
    }
  }

  // helper function to return the value of a memory instruction
  static SSATmp* findValue(IRInstruction* inst) {
    ASSERT(inst != NULL);

    Opcode op = inst->getOpcode();
    if (isLoad(op)) {
      return inst->getDst();
    }
    if (isStore(op)) {
      if (op == StProp || op == StPropNT) {
        return inst->getSrc(2);
      }
      return inst->getSrc(1);
    }
    if (op == Box) {
      return inst->getSrc(0);
    }

    // no support for extracting the value from this instruction, so return NULL
    return NULL;
  }

  // returns true if 'opc' is a load that MemMap handles
  static inline bool isLoad(Opcode opc) {
    // doesn't handle LdMem yet
    return opc == LdLoc || opc == LdRefNR || opc == LdPropNR;
  }

  // returns true if 'opc' is a store that MemMap handles
  static inline bool isStore(Opcode opc) {
    // doesn't handle StMem, StMemNT
    return opc == StRef || opc == StRefNT || opc == StLoc || opc == StLocNT ||
           opc == StProp || opc == StPropNT;
  }
};


} } }


#endif // __HHIR_MEMELIM_H__
