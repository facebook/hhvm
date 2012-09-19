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

#include <util/trace.h>
#include <runtime/vm/translator/asm-x64.h>
#include "ir.h"
#include "dce.h"
#include "ir.h"
#include "memelim.h"
#include "simplifier.h"

namespace HPHP {
namespace VM {
namespace JIT {


static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

static const int Error[] = {
#define OPC(name, hasDst, canCSE, essential, effects, native, consRef,  \
            prodRef, mayModRefs, rematerializable, error)               \
  error,
  IR_OPCODES
  #undef OPC
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
    } else if (Error[op]) {
      // if the function has an exit edge that we don't know anything about
      // (raising an error), then all stores we're currently tracking need to
      // be erased. all stores already declared dead are untouched
      StoreList::iterator it, end;
      for (it = tracking.begin(), end = tracking.end(); it != end; ) {
        StoreList::iterator copy = it;
        ++it;
        if (copy->first->getId() != DEAD) {
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

void optimizeMemoryAccesses(Trace* trace, IRFactory* factory) {
  MemMap mem(factory);

  mem.optimizeMemoryAccesses(trace);
}

} } }
