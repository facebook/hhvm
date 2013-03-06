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
#include "runtime/vm/translator/hopt/opt.h"
#include "runtime/vm/translator/hopt/irfactory.h"
#include "runtime/vm/translator/hopt/simplifier.h"

namespace HPHP {
namespace VM {
namespace JIT {

static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::hhir;

namespace {

class MemMap {
 public:
  MemMap(IRFactory* factory)
    : m_factory(factory)
    , m_liveInsts(factory->numInsts()) {
    assert(factory != nullptr);
  }

  ~MemMap() {
    clearCountedMap(m_unescaped);
    clearCountedMap(m_unknown);
    clearLocalsMap();
    clearCountedMap(m_props);
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
    RefInfo(IRInstruction* inst, bool killValue = false) {
      update(inst, killValue);
    }

    void update(IRInstruction* inst, bool killValue = false) {
      assert(inst != nullptr);
      access = inst;
      value = killValue ? nullptr : findRefValue(inst);
    }

    IRInstruction* access;
    SSATmp* value;
  };

  struct LocInfo : public Counted {
    LocInfo(IRInstruction* inst, bool killValue = false) {
      update(inst, killValue);
    }

    void update(IRInstruction* inst, bool killValue = false) {
      assert(inst != nullptr);
      access = inst;
      value = killValue ? nullptr : findValue(inst);
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
      assert(inst != nullptr);
      assert(off >= 0);
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
      assert(inst != nullptr);

      int offset = inst->getSrc(1)->getValInt();

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

  // map local ids to information about the locals.
  typedef hphp_hash_map<int32_t,LocInfo*> LocalsMap;

  // a mapping of stores to a vector of guarded instructions that follow them.
  // this is used to track where we need to sink partially dead stores.
  typedef std::list<std::pair<IRInstruction*, std::vector<IRInstruction*> > >
    StoreList;

  // updates the memory map given the information from the new instruction
  void processInstruction(IRInstruction* inst, bool isPseudoMain);

  // returns the last memory access of a ref, or an object property. an offset
  // of -1 means that the ssa temp is a ref or a local, otherwise the temp is
  // an object and we're trying to access a property with that offset. if the
  // last access of the ref is not known, then this function returns NULL
  IRInstruction* getLastAccess(SSATmp* ref, int offset = -1);

  // returns the value stored in a memory location if it is still available.
  // the arguments are specified the same way as getLastAccess(). if the value
  // of the ref is not known, then this function returns NULL
  SSATmp* getValue(SSATmp* ref, int offset = -1);

  IRInstruction* lastLocalAccess(int32_t id) const {
    auto it = m_locs.find(id);
    return it == m_locs.end() ? nullptr : it->second->access;
  }

  SSATmp* getLocalValue(int32_t id) const {
    auto it = m_locs.find(id);
    return it == m_locs.end() ? nullptr : it->second->value;
  }

  // kills information in our 'unknown' map
  void killRefInfo(IRInstruction* save);
  // kills information in our 'props' map
  void killPropInfo(IRInstruction* save);

  // escapes a box (and all of its aliases) from the 'unescaped' map into the
  // 'unknown' map
  void escapeRef(SSATmp* ref);

  void optimizeLoad(IRInstruction* inst, int offset = -1);
  void sinkStores(StoreList& stores);

  template <typename V>
  static void clearCountedMap(hphp_hash_map<SSATmp*, V*>& map) {
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

  void clearLocalsMap() {
    for (auto& kv : m_locs) {
      kv.second->dec();
    }
    m_locs.clear();
  }

  // helper function to return the value of a memory instruction
  static SSATmp* findValue(IRInstruction* inst) {
    assert(inst != nullptr);

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
    if (isVectorOp(op)) {
      // Vector instructions may change boxes such that the new value
      // only exists in memory, so there's no SSATmp for it.
      return nullptr;
    }
    if (op == Box) {
      return inst->getSrc(0);
    }

    // no support for extracting the value from this instruction, so return NULL
    return nullptr;
  }

  static SSATmp* findRefValue(IRInstruction* inst) {
    assert(inst != nullptr);
    switch (inst->getOpcode()) {
      case LdRef:
        return inst->getDst();
      case StRef:
      case StRefNT:
        return inst->getSrc(1);
      case Box:
        return inst->getSrc(0);
      default:
        // no support for extracting the value from this instruction
        return nullptr;
    }
  }

  static bool isLoad(Opcode opc) {
    return opc == LdLoc || opc == LdRef || opc == LdProp;
  }

  static bool isStore(Opcode opc) {
    return opc == StRef || opc == StRefNT || opc == StLoc || opc == StLocNT ||
           opc == StProp || opc == StPropNT;
  }

  static bool isVectorOp(Opcode opc) {
    switch (opc) {
      case SetProp:
      case SetElem:
      case SetNewElem:
      case ElemDX:      return true;
      default:          return false;
    }
  }

  bool isLive(IRInstruction* inst) const {
    assert(inst->getIId() < m_liveInsts.size());
    return m_liveInsts.test(inst->getIId());
  }

  void setLive(IRInstruction& inst, bool live) {
    assert(inst.getIId() < m_liveInsts.size());
    m_liveInsts.set(inst.getIId(), live);
  }

private:
  IRFactory* m_factory; // needed to clone instructions for sinking
  RefMap m_unescaped;   // refs that are known not to have escaped
  RefMap m_unknown;     // refs that have escaped or possibly escaped
  LocalsMap m_locs;     // locals
  PropMap m_props;      // property accesses
  boost::dynamic_bitset<> m_liveInsts;  // live/dead bits for each instruction
};

void MemMap::killRefInfo(IRInstruction* save) {
  assert(save != nullptr);

  for (RefMap::iterator it = m_unknown.begin(), end = m_unknown.end();
       it != end; ++it) {
    // if 'save' is a load, then don't kill access info of refs that have a
    // different type than 'save'
    if ((isLoad(save->getOpcode()) || save->getOpcode() == LdMem)) {
      auto saveType = save->getDst()->getType();
      if (it->second->value != nullptr &&
          it->second->value->getType() != saveType &&
          it->second->value->getType().isKnownDataType() &&
          saveType.isKnownDataType()) {
        continue;
      }
    }
    // TODO consider doing the same with the type of the base ref pointer of a
    // store

    // if the last access of the current ref is different than the instruction
    // we're saving, kill its info
    if (it->second->access != save) {
      it->second->access = nullptr;
      if (isStore(save->getOpcode()) ||
          save->getOpcode() == StMem || save->getOpcode() == StMemNT) {
        it->second->value = nullptr;
      }
    }
  }
}

void MemMap::killPropInfo(IRInstruction* save) {
  assert(save != nullptr);

  PropInfoList* propInfoList = nullptr;
  PropMap::iterator find = m_props.find(save->getSrc(0));
  if (find != m_props.end()) {
    propInfoList = find->second;
  }

  // get out the offset only if we know one exists (we might be killing because
  // of a LdMem or StMem), otherwise -1
  Opcode op = save->getOpcode();
  int offset = (op == LdProp || op == StProp || op == StPropNT) ?
               save->getSrc(1)->getValInt() : -1;

  for (PropMap::iterator it = m_props.begin(), end = m_props.end();
       it != end; ++it) {
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
        if ((isLoad(save->getOpcode()) || save->getOpcode() == LdMem) &&
            copy->value != nullptr &&
            copy->value->getType() != save->getDst()->getType() &&
            copy->value->getType().isKnownDataType() &&
            save->getDst()->getType().isKnownDataType()) {
          continue;
        }
        // TODO consider doing the same with the type of the base ref pointer
        // of a store

        if (isStore(save->getOpcode()) ||
            save->getOpcode() == StMem || save->getOpcode() == StMemNT) {
          accesses.erase(copy);
        } else {
          copy->access = nullptr;
        }
      }
    }
  }
}

void MemMap::escapeRef(SSATmp* ref) {
  RefMap::iterator i = m_unescaped.find(ref);
  assert(i != m_unescaped.end());

  RefInfo* info = i->second;

  // find all unescaped refs that alias 'ref' and move them over into 'unknown'
  for (RefMap::iterator it = m_unescaped.begin(), end = m_unescaped.end();
       it != end; ) {
    RefMap::iterator copy = it;
    ++it;
    if (copy->second == info) {
      m_unknown.insert(*copy);
      m_unescaped.erase(copy);
    }
  }
}

void MemMap::processInstruction(IRInstruction* inst, bool isPseudoMain) {
  auto storeLocal = [&](uint32_t locId, SSATmp* val) {
    // if 'val' is an unescaped ref, then we need to escape all refs that
    // alias 'val'
    if (m_unescaped.count(val) > 0) {
      escapeRef(val);
    }

    // storing into a local does not affect the info of any other ref
    auto& info = m_locs[locId];
    const bool killValue = val == nullptr;
    if (info == nullptr) {
      info = new LocInfo(inst, killValue);
    } else {
      info->update(inst, killValue);
    }
  };

  assert(inst != nullptr);

  Opcode op = inst->getOpcode();

  switch (op) {
    case Box: {
      m_unescaped[inst->getDst()] = new RefInfo(inst);
      break;
    }

    // loads update the last access and value of a ref, as well as kill off the
    // access info for any refs that may alias

    case LdLoc: {
      auto id = inst->getExtra<LdLoc>()->locId;

      // locals never alias, so no access info needs to be killed
      if (m_locs.count(id) > 0) {
        m_locs[id]->update(inst);
      } else {
        m_locs[id] = new LocInfo(inst);
      }
      break;
    }
    case Unbox:
    case LdRef: {
      SSATmp* ref = inst->getSrc(0);

      // only need to kill access info of possibly escaped refs
      if (m_unescaped.count(ref) > 0) {
        m_unescaped[ref]->update(inst);
        break;
      }

      if (m_unknown.count(ref) > 0) {
        m_unknown[ref]->update(inst);
      } else {
        m_unknown[ref] = new RefInfo(inst);
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
      if (source->getType().isBoxed()) {
        if (m_unescaped.count(source) > 0) {
          m_unescaped[dest] = m_unescaped[source];
          if (op == IncRef) {
            m_unescaped[dest]->inc();
          }
        } else if (m_unknown.count(source) > 0) {
          m_unknown[dest] = m_unknown[source];
          if (op == IncRef) {
            m_unknown[dest]->inc();
          }
        } else {
          m_unknown[source] = new RefInfo(source->getInstruction());
          m_unknown[dest] = m_unknown[source];
          if (op == IncRef) {
            m_unknown[dest]->inc();
          }
        }
      } else if (source->getType() == Type::Obj) {
        if (m_props.count(source) > 0) {
          m_props[dest] = m_props[source];
          if (op == IncRef) {
            m_props[dest]->inc();
          }
        } else {
          m_props[source] = new PropInfoList;
          m_props[dest] = m_props[source];
          if (op == IncRef) {
            m_props[dest]->inc();
          }
        }
      }
      break;
    }

    // stores update the last access of a ref, and kill last access AND value
    // info of any refs that may alias the source ref

    case StLoc:
    case StLocNT: {
      storeLocal(inst->getExtra<LocalId>()->locId, inst->getSrc(1));
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
      if (m_unescaped.count(ref) > 0) {
        m_unescaped[alias] = m_unescaped[ref];
        m_unescaped[alias]->update(inst);
        m_unescaped.erase(ref);
        break;
      }

      // storing to a possibly escaped ref means we need to kill the info of
      // all other possibly escaped refs that may alias the source ref
      if (m_unknown.count(ref) > 0) {
        m_unknown[alias] = m_unknown[ref];
        m_unknown[alias]->update(inst);
        m_unknown.erase(ref);
      } else {
        m_unknown[alias] = new RefInfo(inst);
      }

      killRefInfo(inst);
      break;
    }
    case LdMem:
    case StMem:
    case StMemNT: {
      // might have trampled over the value of a box if it had been stored as
      // the property
      killRefInfo(inst);
      killPropInfo(inst);
      break;
    }
    case LdProp:
    case StProp:
    case StPropNT: {
      SSATmp* obj = inst->getSrc(0);

      // if we're storing out an unescaped ref, then it has now escaped
      if (op != LdProp && m_unescaped.count(inst->getSrc(2)) > 0) {
        escapeRef(inst->getSrc(2));
      }

      if (m_props.count(obj) > 0) {
        m_props[obj]->update(inst);
      } else {
        PropInfoList* list = new PropInfoList;
        int offset = inst->getSrc(1)->getValInt();
        list->accesses.push_back(PropInfo(inst, offset));
        m_props[obj] = list;
      }

      killPropInfo(inst);
      break;
    }
    case SetProp:
    case SetElem:
    case SetNewElem:
    case ElemDX: {
      VectorEffects::get(inst,
                         /* storeLocValue callback */
                         storeLocal,
                         /* setLocType callback. Erases the value for now. */
                         [&](uint32_t id, Type t) { storeLocal(id, nullptr); });
      break;
    }
    case DecRef:
    case DecRefNZ: {
      SSATmp* ref = inst->getSrc(0);
      Type ty = inst->getSrc(0)->getType();

      // decref of a string has no side effects
      if (ty.isString()) {
        break;
      }

      int count = 0;

      if (m_unescaped.count(ref) > 0) {
        count = m_unescaped[ref]->dec();
        m_unescaped.erase(ref);
      } else if (m_unknown.count(ref) > 0) {
        count = m_unknown[ref]->dec();
        m_unknown.erase(ref);
      } else if (m_props.count(ref) > 0) {
        count = m_props[ref]->dec();
        m_props.erase(ref);
      }
      // don't kill info if we know we haven't destroyed the object, or if we
      // know that there won't be any destructors being called
      // DecRefNZ can't call a destructor.
      if (!ty.canRunDtor() || count != 0 || op == DecRefNZ) {
        break;
      }
      // otherwise fall through to default case. a DecRef of an Obj{&}/Arr{&}
      // can modify refs if the source hits zero and runs a destructor
    }
    default: {
      if (inst->mayModifyRefs()) {

        // escape any boxes that are on the right hand side of the current
        // instruction
        RefMap::iterator end = m_unescaped.end();
        for (SSATmp* src : inst->getSrcs()) {
          RefMap::iterator find = m_unescaped.find(src);
          if (find != end) {
            escapeRef(find->first);
            break;
          }
        }

        clearCountedMap(m_unknown);
        clearCountedMap(m_props);
        clearLocalsMap();
      }
      break;
    }
  }
}

#define MEMMAP_GET(FIELD)                                                     \
  assert(offset >= -1);                                                       \
  /* check for property accesses */                                           \
  if (offset != -1) {                                                         \
    PropMap::iterator it = m_props.find(ref);                                 \
    if (it == m_props.end()) {                                                \
      return nullptr;                                                         \
    }                                                                         \
    PropList& list = it->second->accesses;                                    \
    for (PropList::iterator i = list.begin(), e = list.end(); i != e; ++i) {  \
      if (i->offset == offset) {                                              \
        return i->FIELD;                                                      \
      }                                                                       \
    }                                                                         \
    return nullptr;                                                           \
  }                                                                           \
  /* otherwise, check all of our ref maps */                                  \
  RefMap::iterator it = m_unescaped.find(ref);                                \
  if (it != m_unescaped.end()) {                                              \
    return it->second->FIELD;                                                 \
  }                                                                           \
  it = m_unknown.find(ref);                                                   \
  if (it != m_unknown.end()) {                                                \
    return it->second->FIELD;                                                 \
  }                                                                           \
  return nullptr;                                                             \


IRInstruction* MemMap::getLastAccess(SSATmp* ref, int offset) {
  MEMMAP_GET(access)
}

SSATmp* MemMap::getValue(SSATmp* ref, int offset) {
  MEMMAP_GET(value)
}

void MemMap::optimizeMemoryAccesses(Trace* trace) {
  if (hasInternalFlow(trace)) {
    // This algorithm only works with linear traces.
    // TODO t2066994: reset state after each block, at least.
    return;
  }
  StoreList tracking;

  const Func* curFunc = nullptr;

  for (Block* block : trace->getBlocks()) {
    for (IRInstruction& inst : *block) {
      if (inst.getOpcode() == Marker) {
        curFunc = inst.getExtra<Marker>()->func;
      }
      // initialize each instruction as live
      setLive(inst, true);

      int offset = -1;
      Opcode op = inst.getOpcode();

      if (isLoad(op)) {
        if (op == LdProp) {
          offset = inst.getSrc(1)->getValInt();
        }
        // initialize each instruction as live
        setLive(inst, true);

        optimizeLoad(&inst, offset);
      } else if (isStore(op)) {
        if (op == StProp || op == StPropNT) {
          offset = inst.getSrc(1)->getValInt();
        }

        // if we see a store, first check if its last available access is a store
        // if it is, then the last access is a dead store
        auto access = inst.getOpcode() == StLoc || inst.getOpcode() == StLocNT
          ? lastLocalAccess(inst.getExtra<LocalId>()->locId)
          : getLastAccess(inst.getSrc(0), offset);
        if (access && isStore(access->getOpcode())) {
          // if a dead St* is followed by a St*NT, then the second store needs to
          // now write in the type because the first store will be removed
          if (access->getOpcode() == StProp && op == StPropNT) {
            inst.setOpcode(StProp);
          } else if (access->getOpcode() == StLoc && op == StLocNT) {
            inst.setOpcode(StLoc);
          } else if (access->getOpcode() == StRef && op == StRefNT) {
            inst.setOpcode(StRef);
          }

          setLive(*access, false);
        }

        // start tracking the current store
        tracking.push_back(std::make_pair(&inst, std::vector<IRInstruction*>()));
      } else if (inst.mayRaiseError()) {
        // if the function has an exit edge that we don't know anything about
        // (raising an error), then all stores we're currently tracking need to
        // be erased. all stores already declared dead are untouched
        StoreList::iterator it, end;
        for (it = tracking.begin(), end = tracking.end(); it != end; ) {
          StoreList::iterator copy = it;
          ++it;
          if (isLive(copy->first)) {
            // XXX: t1779667
            tracking.erase(copy);
          }
        }
      }

      // if the current instruction is guarded, make sure all of our stores that
      // are not yet dead know about it
      if (inst.getTaken()) {
        for (auto& entry : tracking) {
          if (isLive(entry.first)) {
            entry.second.push_back(&inst);
          }
        }
      }
      Simplifier::copyProp(&inst);
      processInstruction(&inst, curFunc && curFunc->isPseudoMain());
    }
  }

  sinkStores(tracking);

  // kill the dead stores
  removeDeadInstructions(trace, m_liveInsts);
}

void MemMap::optimizeLoad(IRInstruction* inst, int offset) {
  // check if we still know the value at this memory location. if we do,
  // then replace the load with a Mov
  auto value = inst->getOpcode() == LdLoc
    ? getLocalValue(inst->getExtra<LocalId>()->locId)
    : getValue(inst->getSrc(0), offset);
  if (value == nullptr) {
    return;
  }

  Type instTy = inst->getDst()->getType();
  Type valTy = value->getType();

  // check for loads that have a guard that will definitely fail
  if (inst->getTaken() && instTy.not(valTy)) {
    return inst->convertToJmp();
  }

  Opcode op = inst->getOpcode();

  // fix the instruction's arguments and rip off its label if it had one
  inst->setSrc(0, value);
  if (op == LdProp) {
    inst->setSrc(1, nullptr);
    inst->setNumSrcs(1);
  } else {
    assert(inst->getNumSrcs() == 1);
  }
  inst->setTaken(nullptr);

  // convert the instruction into a Mov with the known value
  inst->convertToMov();
}

void MemMap::sinkStores(StoreList& stores) {
  // sink dead stores into exit edges that occur between the dead store and the
  // next store
  StoreList::reverse_iterator it, end;
  for (it = stores.rbegin(), end = stores.rend(); it != end; ++it) {
    IRInstruction* store = it->first;

    if (isLive(store)) continue;

    for (IRInstruction* guard : it->second) {
      Block* exit = guard->getTaken();
      exit->prepend(store->clone(m_factory));
    }

    // StRefs cannot just be removed, they have to be converted into Movs
    // as the destination of the StRef still has the DecRef attached to it.
    if (store->getOpcode() == StRef || store->getOpcode() == StRefNT) {
      store->setOpcode(Mov);
      store->setSrc(1, nullptr);
      store->setNumSrcs(1);
      setLive(*store, true);
    }
  }
}

}

void optimizeMemoryAccesses(Trace* trace, IRFactory* factory) {
  MemMap(factory).optimizeMemoryAccesses(trace);
}

} } }
