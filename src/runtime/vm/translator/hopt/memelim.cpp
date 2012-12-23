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

#include "util/trace.h"
#include "util/asm-x64.h"
#include "runtime/base/util/countable.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/dce.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/memelim.h"
#include "runtime/vm/translator/hopt/simplifier.h"

namespace HPHP {
namespace VM {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

namespace {

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

void MemMap::killRefInfo(IRInstruction* save) {
  ASSERT(save != NULL);

  RefMap::iterator it, end;
  for (it = unknown.begin(), end = unknown.end(); it != end; ++it) {
    // if 'save' is a load, then don't kill access info of refs that have a
    // different type than 'save'
    if ((isLoad(save->getOpcode()) || save->getOpcode() == LdMemNR)) {
      if (it->second->value != NULL &&
          it->second->value->getType() != save->getType() &&
          Type::isStaticallyKnown(it->second->value->getType()) &&
          Type::isStaticallyKnown(save->getType())) {
        continue;
      }
    }
    // TODO consider doing the same with the type of the base ref pointer of a
    // store

    // if the last access of the current ref is different than the instruction
    // we're saving, kill its info
    if (it->second->access != save) {
      it->second->access = NULL;
      if (isStore(save->getOpcode()) ||
          save->getOpcode() == StMem || save->getOpcode() == StMemNT) {
        it->second->value = NULL;
      }
    }
  }
}

void MemMap::killPropInfo(IRInstruction* save) {
  ASSERT(save != NULL);

  PropInfoList* propInfoList = NULL;
  PropMap::iterator find = props.find(save->getSrc(0));
  if (find != props.end()) {
    propInfoList = find->second;
  }

  // get out the offset only if we know one exists (we might be killing because
  // of a LdMem or StMem), otherwise -1
  Opcode op = save->getOpcode();
  int offset = (op == LdPropNR || op == StProp || op == StPropNT) ?
               save->getSrc(1)->getConstValAsInt() : -1;

  PropMap::iterator it, end;
  for (it = props.begin(), end = props.end(); it != end; ++it) {
    // ignore info blocks that alias the source object in 'save'. we've already
    // updated their info
    if (it->second == propInfoList) {
      continue;
    }

    PropList& accesses = it->second->accesses;
    for (PropList::iterator i = accesses.begin(), e = accesses.end(); i != e;) {
      PropList::iterator copy = i;
      ++i;
      // only kill info for properties that are at the same offset as the
      // property that got modified in 'save'
      if (offset == -1 || (copy->offset == offset && copy->access != save)) {
        // if 'save' is a load, then don't kill access info of properties that
        // have a different type than 'save'
        if ((isLoad(save->getOpcode()) || save->getOpcode() == LdMemNR) &&
            copy->value != NULL &&
            copy->value->getType() != save->getType() &&
            Type::isStaticallyKnown(copy->value->getType()) &&
            Type::isStaticallyKnown(save->getType())) {
          continue;
        }
        // TODO consider doing the same with the type of the base ref pointer
        // of a store

        if (isStore(save->getOpcode()) ||
            save->getOpcode() == StMem || save->getOpcode() == StMemNT) {
          accesses.erase(copy);
        } else {
          copy->access = NULL;
        }
      }
    }
  }
}

void MemMap::escapeRef(SSATmp* ref) {
  RefMap::iterator i = unescaped.find(ref);
  ASSERT(i != unescaped.end());

  RefInfo* info = i->second;

  // find all unescaped refs that alias 'ref' and move them over into 'unknown'
  RefMap::iterator it, end;
  for (it = unescaped.begin(), end = unescaped.end(); it != end; ) {
    RefMap::iterator copy = it;
    ++it;

    if (copy->second == info) {
      unknown.insert(*copy);
      unescaped.erase(copy);
    }
  }
}

void MemMap::processInstruction(IRInstruction* inst) {
  ASSERT(inst != NULL);

  Opcode op = inst->getOpcode();

  switch (op) {
    case Box: {
      unescaped[inst->getDst()] = new RefInfo(inst);
      break;
    }

    // loads update the last access and value of a ref, as well as kill off the
    // access info for any refs that may alias

    case LdLoc: {
      SSATmp* home = inst->getSrc(0);

      // locals never alias, so no access info needs to be killed
      if (locs.count(home) > 0) {
        locs[home]->update(inst);
      } else {
        locs[home] = new RefInfo(inst);
      }
      break;
    }
    case LdRefNR: {
      SSATmp* ref = inst->getSrc(0);

      // only need to kill access info of possibly escaped refs
      if (unescaped.count(ref) > 0) {
        unescaped[ref]->update(inst);
        break;
      }

      if (unknown.count(ref) > 0) {
        unknown[ref]->update(inst);
      } else {
        unknown[ref] = new RefInfo(inst);
      }

      killRefInfo(inst);
      break;
    }

    // Movs and IncRefs create new aliases that we want to track

    case Mov:
    case IncRef: {
      SSATmp* dest = inst->getDst();
      SSATmp* source = inst->getSrc(0);

      // figure out which map the new alias is supposed to be inserted into
      if (Type::isBoxed(source->getType())) {
        if (unescaped.count(source) > 0) {
          unescaped[dest] = unescaped[source];
          if (op == IncRef) {
            unescaped[dest]->inc();
          }
        } else if (unknown.count(source) > 0) {
          unknown[dest] = unknown[source];
          if (op == IncRef) {
            unknown[dest]->inc();
          }
        } else {
          unknown[source] = new RefInfo(source->getInstruction());
          unknown[dest] = unknown[source];
          if (op == IncRef) {
            unknown[dest]->inc();
          }
        }
      } else if (source->getType() == Type::Obj) {
        if (props.count(source) > 0) {
          props[dest] = props[source];
          if (op == IncRef) {
            props[dest]->inc();
          }
        } else {
          props[source] = new PropInfoList;
          props[dest] = props[source];
          if (op == IncRef) {
            props[dest]->inc();
          }
        }
      }
      break;
    }

    // stores update the last access of a ref, and kill last access AND value
    // info of any refs that may alias the source ref

    case StLoc:
    case StLocNT: {
      SSATmp* home = inst->getSrc(0);
      SSATmp* val = inst->getSrc(1);

      // if 'val' is an unescaped ref, then we need to escape all refs that
      // alias 'val'
      if (unescaped.count(val) > 0) {
        escapeRef(val);
      }

      // storing into a local does not affect the info of any other ref
      if (locs.count(home) > 0) {
        locs[home]->update(inst);
      } else {
        locs[home] = new RefInfo(inst);
      }
      break;
    }
    case StRef:
    case StRefNT: {
      SSATmp* ref = inst->getSrc(0);
      SSATmp* alias = inst->getDst();

      // StRef* instructions create a new alias that doesn't affect our tracked
      // ref count. after a StRef, the source temp is no longer used, so it just
      // gets replaced with the dest temp

      // we know all the refs that alias an unescaped ref, so just update their
      // value
      if (unescaped.count(ref) > 0) {
        unescaped[alias] = unescaped[ref];
        unescaped[alias]->update(inst);
        unescaped.erase(ref);
        break;
      }

      // storing to a possibly escaped ref means we need to kill the info of
      // all other possibly escaped refs that may alias the source ref
      if (unknown.count(ref) > 0) {
        unknown[alias] = unknown[ref];
        unknown[alias]->update(inst);
        unknown.erase(ref);
      } else {
        unknown[alias] = new RefInfo(inst);
      }

      killRefInfo(inst);
      break;
    }
    case LdMemNR:
    case StMem:
    case StMemNT: {
      // might have trampled over the value of a box if it had been stored as
      // the property
      killRefInfo(inst);
      killPropInfo(inst);
      break;
    }
    case LdPropNR:
    case StProp:
    case StPropNT: {
      SSATmp* obj = inst->getSrc(0);

      // if we're storing out an unescaped ref, then it has now escaped
      if (op != LdPropNR && unescaped.count(inst->getSrc(2)) > 0) {
        escapeRef(inst->getSrc(2));
      }

      if (props.count(obj) > 0) {
        props[obj]->update(inst);
      } else {
        PropInfoList* list = new PropInfoList;
        int offset = inst->getSrc(1)->getConstValAsInt();
        list->accesses.push_back(PropInfo(inst, offset));
        props[obj] = list;
      }

      killPropInfo(inst);
      break;
    }
    case DecRef:
    case DecRefNZ: {
      SSATmp* ref = inst->getSrc(0);

      // DecRefs default to types of Cell& for boxed types, update them if we
      // have their type information
      if (Type::isBoxed(ref->getType())) {
        SSATmp* val = getValue(ref);
        if (val != NULL) {
          inst->setType(val->getType());
        }
      }

      Type::Tag ty = inst->getType();

      // decref of a string has no side effects
      if (Type::isString(ty)) {
        break;
      }

      int count = 0;

      if (unescaped.count(ref) > 0) {
        count = unescaped[ref]->dec();
        unescaped.erase(ref);
      } else if (unknown.count(ref) > 0) {
        count = unknown[ref]->dec();
        unknown.erase(ref);
      } else if (props.count(ref) > 0) {
        count = props[ref]->dec();
        props.erase(ref);
      }
      // don't kill info if we know we haven't destroyed the object, or if we
      // know that there won't be any destructors being called
      // DecRefNZ can't call a destructor.
      if (!Type::canRunDtor(ty) || count != 0 || op == DecRefNZ) {
        break;
      }
      // otherwise fall through to default case. a DecRef of an Obj{&}/Arr{&}
      // can modify refs if the source hits zero and runs a destructor
    }
    default: {
      if (inst->mayModifyRefs()) {

        // escape any boxes that are on the right hand side of the current
        // instruction
        RefMap::iterator end = unescaped.end();
        for (uint32 i = 0; i < inst->getNumSrcs(); ++i) {
          RefMap::iterator find = unescaped.find(inst->getSrc(i));
          if (find != end) {
            escapeRef(find->first);
            break;
          }
        }

        clearCountedMap(unknown);
        clearCountedMap(props);

        // TODO always killing locals on an instruction that can modify refs
        // is too conservative
        clearCountedMap(locs);
      }
      break;
    }
  }
}

#define MEMMAP_GET(FIELD)                                                     \
  ASSERT(offset >= -1);                                                       \
  /* check for property accesses */                                           \
  if (offset != -1) {                                                         \
    PropMap::iterator it = props.find(ref);                                   \
    if (it == props.end()) {                                                  \
      return NULL;                                                            \
    }                                                                         \
    PropList& list = it->second->accesses;                                    \
    for (PropList::iterator i = list.begin(), e = list.end(); i != e; ++i) {  \
      if (i->offset == offset) {                                              \
        return i->FIELD;                                                      \
      }                                                                       \
    }                                                                         \
    return NULL;                                                              \
  }                                                                           \
  /* otherwise, check all of our ref maps */                                  \
  RefMap::iterator it = unescaped.find(ref);                                  \
  if (it != unescaped.end()) {                                                \
    return it->second->FIELD;                                                 \
  }                                                                           \
  it = unknown.find(ref);                                                     \
  if (it != unknown.end()) {                                                  \
    return it->second->FIELD;                                                 \
  }                                                                           \
  it = locs.find(ref);                                                        \
  if (it != locs.end()) {                                                     \
    return it->second->FIELD;                                                 \
  }                                                                           \
  return NULL;                                                                \

IRInstruction* MemMap::getLastAccess(SSATmp* ref, int offset) {
  MEMMAP_GET(access)
}

SSATmp* MemMap::getValue(SSATmp* ref, int offset) {
  MEMMAP_GET(value)
}

void MemMap::optimizeMemoryAccesses(Trace* trace) {
  StoreList tracking;

  IRInstruction::List& insts = trace->getInstructionList();
  IRInstruction::Iterator it, end;
  for (it = insts.begin(), end = insts.end(); it != end; ++it) {
    IRInstruction* inst = *it;

    // initialize each instruction as live
    inst->setId(LIVE);

    int offset = -1;
    Opcode op = inst->getOpcode();

    if (isLoad(op)) {
      if (op == LdPropNR) {
        offset = inst->getSrc(1)->getConstValAsInt();
      }

      optimizeLoad(inst, offset);
    } else if (isStore(op)) {
      if (op == StProp || op == StPropNT) {
        offset = inst->getSrc(1)->getConstValAsInt();
      }

      // if we see a store, first check if its last available access is a store
      // if it is, then the last access is a dead store
      IRInstruction* access = getLastAccess(inst->getSrc(0), offset);
      if (access != NULL && isStore(access->getOpcode())) {
        // if a dead St* is followed by a St*NT, then the second store needs to
        // now write in the type because the first store will be removed
        if (access->getOpcode() == StProp && op == StPropNT) {
          inst->setOpcode(StProp);
        } else if (access->getOpcode() == StLoc && op == StLocNT) {
          inst->setOpcode(StLoc);
        } else if (access->getOpcode() == StRef && op == StRefNT) {
          inst->setOpcode(StRef);
        }

        access->setId(DEAD);
      }

      // start tracking the current store
      tracking.push_back(std::make_pair(inst, std::vector<IRInstruction*>()));
    } else if (opcodeHasFlags(op, MayRaiseError)) {
      // if the function has an exit edge that we don't know anything about
      // (raising an error), then all stores we're currently tracking need to
      // be erased. all stores already declared dead are untouched
      StoreList::iterator it, end;
      for (it = tracking.begin(), end = tracking.end(); it != end; ) {
        StoreList::iterator copy = it;
        ++it;
        if (copy->first->getId() != DEAD) {
          // XXX: t1779667
          tracking.erase(copy);
        }
      }
    }

    // if the current instruction is guarded, make sure all of our stores that
    // are not yet dead know about it
    if (inst->getLabel() != NULL) {
      StoreList::iterator it, end;
      for (it = tracking.begin(), end = tracking.end(); it != end; ++it) {
        if (it->first->getId() != DEAD) {
          it->second.push_back(inst);
        }
      }
    }

    Simplifier::copyProp(inst);
    processInstruction(inst);
  }

  sinkStores(tracking);

  // kill the dead stores
  removeDeadInstructions(trace);
}

void MemMap::optimizeLoad(IRInstruction* inst, int offset) {
  // check if we still know the value at this memory location. if we do,
  // then replace the load with a Mov
  SSATmp* value = getValue(inst->getSrc(0), offset);

  if (value == NULL) {
    return;
  }

  Type::Tag instTy = inst->getType();
  Type::Tag valTy = value->getType();

  // check for loads that have a guard and will fail it
  if (inst->getLabel() != NULL && valTy != instTy) {
    if (!(Type::isString(valTy) && Type::isString(instTy)) &&
        Type::isStaticallyKnown(valTy) && Type::isStaticallyKnown(instTy)) {
      inst->setOpcode(Jmp_);
      inst->m_numSrcs = 0;
      inst->setDst(NULL);
      return;
    }
  }

  Opcode op = inst->getOpcode();

  // fix the instruction's arguments and rip off its label if it had one
  inst->setSrc(0, value);
  if (op == LdPropNR) {
    inst->setSrc(1, NULL);
    inst->m_numSrcs = 1;
  } else {
    ASSERT(inst->getNumSrcs() == 1);
  }
  inst->setLabel(NULL);

  // convert the instruction into a Mov with the known value
  inst->setOpcode(Mov);
}

void MemMap::sinkStores(StoreList& stores) {
  // sink dead stores into exit edges that occur between the dead store and the
  // next store
  StoreList::reverse_iterator it, end;
  for (it = stores.rbegin(), end = stores.rend(); it != end; ++it) {
    IRInstruction* store = it->first;

    if (store->getId() != DEAD) {
      continue;
    }

    std::vector<IRInstruction*>::iterator i, e;
    for (i = it->second.begin(), e = it->second.end(); i != e; ++i) {
      IRInstruction* guard = *i;

      IRInstruction* clone = store->clone(factory);
      if (store->getDst() != NULL) {
        factory->getSSATmp(clone);
      }

      guard->getLabel()->getTrace()->prependInstruction(clone);
    }

    // StRefs cannot just be removed, they have to be converted into Movs
    // as the destination of the StRef still has the DecRef attached to it.
    if (store->getOpcode() == StRef || store->getOpcode() == StRefNT) {
      store->setOpcode(Mov);
      store->setSrc(1, NULL);
      store->m_numSrcs = 1;
      store->setId(LIVE);
    }
  }
}

}

void optimizeMemoryAccesses(Trace* trace, IRFactory* factory) {
  MemMap mem(factory);

  mem.optimizeMemoryAccesses(trace);
}

} } }
