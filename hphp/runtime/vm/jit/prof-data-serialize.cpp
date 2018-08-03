/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/prof-data-serialize.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/std/ext_std_closure.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/jit/array-kind-profile.h"
#include "hphp/runtime/vm/jit/array-offset-profile.h"
#include "hphp/runtime/vm/jit/call-target-profile.h"
#include "hphp/runtime/vm/jit/cls-cns-profile.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/meth-profile.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/profile-refcount.h"
#include "hphp/runtime/vm/jit/release-vv-profile.h"
#include "hphp/runtime/vm/jit/switch-profile.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/type-profile.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/build-info.h"
#include "hphp/util/process.h"

#include <folly/portability/Unistd.h>
#include <folly/String.h>

namespace HPHP { namespace jit {
//////////////////////////////////////////////////////////////////////

namespace {
//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

StaticString s_invoke("__invoke");
StaticString s_86ctor("86ctor");
StaticString s_86pinit("86pinit");
StaticString s_86sinit("86sinit");
StaticString s_86linit("86linit");

constexpr uint32_t k86pinitSlot = 0x80000000u;
constexpr uint32_t k86sinitSlot = 0x80000001u;
constexpr uint32_t k86linitSlot = 0x80000002u;

template<typename F>
auto deserialize(ProfDataDeserializer&ser, F&& f) -> decltype(f()) {
  using T = decltype(f());
  auto const ptr = read_raw<uintptr_t>(ser);
  if (!ptr) return T{};
  if (ptr & 1) {
    auto& ent = ser.getEnt(reinterpret_cast<T>(ptr - 1));
    assertx(!ent);
    ent = f();
    ITRACE(3, "0x{:08x} => 0x{:08x}\n",
           ptr - 1, reinterpret_cast<uintptr_t>(ent));
    assertx(ent);
    return ent;
  }
  auto const ent = ser.getEnt(reinterpret_cast<T>(ptr));
  assertx(ent);
  return ent;
}

void write_serialized_ptr(ProfDataSerializer& ser, const void* p) {
  auto const ptr_as_int = reinterpret_cast<uintptr_t>(p);
  assertx(!(ptr_as_int & 1));
  write_raw(ser, ptr_as_int | 1);
}

void write_raw_ptr(ProfDataSerializer& ser, const void* p) {
  auto const ptr_as_int = reinterpret_cast<uintptr_t>(p);
  assertx(!(ptr_as_int & 1));
  write_raw(ser, ptr_as_int);
}

/*
 * Helper functions so that the function passed to write_container can
 * take a serializer as a parameter, or skip it (eg if its a lambda
 * which is already capturing the serializer).
 */
template<typename S, typename T, typename F>
auto call(F& f, S& ser, const T& t) -> decltype(f(ser, t)) {
  return f(ser, t);
}

template<typename S, typename T, typename F>
auto call(F& f, S&, const T& t) -> decltype(f(t)) {
  return f(t);
}

template<typename C, typename F>
void write_container(ProfDataSerializer& ser, const C& cont, F f) {
  write_raw(ser, safe_cast<uint32_t>(cont.size()));
  for (auto const &elm : cont) {
    call(f, ser, elm);
  }
}

template<typename F>
void read_container(ProfDataDeserializer& ser, F f) {
  auto sz = read_raw<uint32_t>(ser);
  while (sz--) { f(); }
}

void write_srckey(ProfDataSerializer& ser, SrcKey sk) {
  ITRACE(2, "SrcKey>\n");
  if (ser.serialize(sk.func())) {
    Trace::Indent _i;
    write_raw(ser, uintptr_t(-1));
    write_func(ser, sk.func());
  }
  write_raw(ser, sk.toAtomicInt());
  ITRACE(2, "SrcKey: {}\n", show(sk));
}

SrcKey read_srckey(ProfDataDeserializer& ser) {
  ITRACE(2, "SrcKey>\n");
  auto orig = read_raw<SrcKey::AtomicInt>(ser);
  if (orig == uintptr_t(-1)) {
    Trace::Indent _i;
    read_func(ser);
    orig = read_raw<SrcKey::AtomicInt>(ser);
  }
  auto const id = SrcKey::fromAtomicInt(orig).funcID();
  assertx(uint32_t(orig) == id);
  auto const sk = SrcKey::fromAtomicInt(orig - id + ser.getFid(id));
  ITRACE(2, "SrcKey: {}\n", show(sk));
  return sk;
}

void write_type(ProfDataSerializer& ser, Type t) {
  t.serialize(ser);
}

Type read_type(ProfDataDeserializer& ser) {
  return Type::deserialize(ser);
}

void write_typed_location(ProfDataSerializer& ser,
                          const RegionDesc::TypedLocation& loc) {
  write_raw(ser, loc.location);
  write_type(ser, loc.type);
}

void write_guarded_location(ProfDataSerializer& ser,
                            const RegionDesc::GuardedLocation& loc) {
  write_raw(ser, loc.location);
  write_type(ser, loc.type);
  write_raw(ser, loc.category);
}

RegionDesc::TypedLocation read_typed_location(ProfDataDeserializer& ser) {
  auto const location = read_raw<Location>(ser);
  auto const type = read_type(ser);
  return { location, type };
}

RegionDesc::GuardedLocation read_guarded_location(ProfDataDeserializer& ser) {
  auto const location = read_raw<Location>(ser);
  auto const type = read_type(ser);
  auto const cat = read_raw<DataTypeCategory>(ser);
  return { location, type, cat };
}

void write_global_array_map(ProfDataSerializer& ser) {
  write_container(ser, globalArrayTypeTable(),
                  write_raw<const RepoAuthType::Array*>);
}

void read_global_array_map(ProfDataDeserializer& ser) {
  auto sz DEBUG_ONLY = read_raw<uint32_t>(ser);
  assertx(sz == globalArrayTypeTable().size());
  for (auto arr : globalArrayTypeTable()) {
    auto const orig = read_raw<const RepoAuthType::Array*>(ser);
    ser.recordRat(orig, arr);
  }
}

void write_region_block(ProfDataSerializer& ser,
                        const RegionDesc::BlockPtr& block) {
  write_raw(ser, block->id());
  write_srckey(ser, block->start());
  write_raw(ser, block->length());
  write_raw(ser, block->initialSpOffset());
  write_raw(ser, block->profTransID());
  write_container(ser, block->typePredictions(), write_typed_location);
  write_container(ser, block->typePreConditions(), write_guarded_location);
  write_container(ser, block->knownFuncs(),
                  [] (ProfDataSerializer& s,
                      std::pair<SrcKey, const Func*> knownFunc) {
                    write_srckey(s, knownFunc.first);
                    write_func(s, knownFunc.second);
                  });
  write_container(ser, block->postConds().changed, write_typed_location);
  write_container(ser, block->postConds().refined, write_typed_location);
}

RegionDesc::BlockPtr read_region_block(ProfDataDeserializer& ser) {
  auto const id = read_raw<RegionDesc::BlockId>(ser);
  auto const start = read_srckey(ser);
  auto const length = read_raw<int>(ser);
  auto const initialSpOffset = read_raw<FPInvOffset>(ser);

  auto const block = std::make_shared<RegionDesc::Block>(id,
                                                         start.func(),
                                                         start.resumeMode(),
                                                         start.hasThis(),
                                                         start.offset(),
                                                         length,
                                                         initialSpOffset);

  block->setProfTransID(read_raw<TransID>(ser));

  read_container(ser,
                 [&] {
                   block->addPredicted(read_typed_location(ser));
                 });

  read_container(ser,
                 [&] {
                   block->addPreCondition(read_guarded_location(ser));
                 });

  read_container(ser,
                 [&] {
                   auto const sk = read_srckey(ser);
                   auto const func = read_func(ser);
                   block->setKnownFunc(sk, func);
                 });

  PostConditions postConds;
  read_container(ser,
                 [&] {
                   postConds.changed.push_back(read_typed_location(ser));
                 });
  read_container(ser,
                 [&] {
                   postConds.refined.push_back(read_typed_location(ser));
                 });
  block->setPostConds(postConds);
  return block;
}

void write_region_desc(ProfDataSerializer& ser, const RegionDesc* rd) {
  write_container(ser, rd->blocks(), write_region_block);
  write_container(ser, findPredTrans(*rd, profData()),
                  [&] (RegionDesc::BlockId id) {
                    write_raw(ser, id);
                  });
  write_container(ser, rd->blocks(),
                  [&] (const RegionDesc::BlockPtr& b) {
                    auto const bid = b->id();
                    write_raw(ser, bid);
                    assertx(rd->succs(bid).empty());
                    assertx(rd->preds(bid).empty());
                    assertx(rd->merged(bid).empty());
                    auto const pr = rd->prevRetrans(bid);
                    write_raw(ser, pr ? pr.value() : kInvalidTransID);
                    auto const nr = rd->nextRetrans(bid);
                    write_raw(ser, nr ? nr.value() : kInvalidTransID);
                  });
  write_type(ser, rd->inlineCtxType());
  write_container(ser, rd->inlineInputTypes(), write_type);
}

RegionDescPtr read_region_desc(ProfDataDeserializer& ser) {
  auto ret = std::make_shared<RegionDesc>();
  read_container(ser, [&] { ret->addBlock(read_region_block(ser)); });
  RegionDesc::BlockIdSet incoming;
  read_container(ser,
                 [&] { incoming.insert(read_raw<RegionDesc::BlockId>(ser)); });
  ret->incoming(std::move(incoming));

  read_container(ser,
                 [&] {
                   auto const id = read_raw<RegionDesc::BlockId>(ser);
                   auto const pr = read_raw<RegionDesc::BlockId>(ser);
                   auto const nr = read_raw<RegionDesc::BlockId>(ser);
                   if (pr != kInvalidTransID) {
                     ret->setNextRetrans(pr, id);
                   }
                   if (nr != kInvalidTransID) {
                     ret->setNextRetrans(id, nr);
                   }
                 });
  auto const ty = read_type(ser);
  std::vector<Type> args;
  read_container(ser,
                 [&] {
                   args.push_back(read_type(ser));
                 });
  ret->setInlineContext(ty, args);
  return ret;
}

void write_prof_trans_rec(ProfDataSerializer& ser,
                          const ProfTransRec* ptr,
                          ProfData* pd) {
  if (!ptr) return write_raw(ser, TransKind{});
  write_raw(ser, ptr->kind());
  write_srckey(ser, ptr->srcKey());
  if (ptr->kind() == TransKind::Profile) {
    write_raw(ser, ptr->lastBcOff());
    write_region_desc(ser, ptr->region().get());
  } else {
    write_raw(ser, ptr->prologueArgs());

    auto lock = ptr->lockCallerList();
    std::vector<TransID> callers;
    auto addCaller = [&] (TCA caller) {
      if (!tc::isProfileCodeAddress(caller)) return;
      auto const callerTransId = pd->jmpTransID(caller);
      assertx(callerTransId != kInvalidTransID);
      callers.push_back(callerTransId);
    };
    for (auto const caller : ptr->mainCallers()) addCaller(caller);
    for (auto const caller : ptr->guardCallers()) addCaller(caller);
    write_container(ser, callers, write_raw<TransID>);
  }
}

std::unique_ptr<ProfTransRec> read_prof_trans_rec(ProfDataDeserializer& ser) {
  auto const kind = read_raw<TransKind>(ser);
  static_assert(TransKind::Profile != TransKind{} &&
                TransKind::ProfPrologue != TransKind{},
                "Profile and ProfPrologue must not be zero");
  if (kind == TransKind{}) return nullptr;
  auto const sk = read_srckey(ser);
  if (kind == TransKind::Profile) {
    auto const lastBcOff = read_raw<Offset>(ser);
    auto const region = read_region_desc(ser);
    return std::make_unique<ProfTransRec>(lastBcOff, sk, region);
  }

  auto ret = std::make_unique<ProfTransRec>(sk, read_raw<int>(ser));

  read_container(ser,
                 [&] {
                   ret->profCallers().push_back(read_raw<TransID>(ser));
                 });

  return ret;
}

bool write_type_alias(ProfDataSerializer& ser, const TypeAliasReq* td);

bool write_type_alias_or_class(ProfDataSerializer& ser, const NamedEntity* ne) {
  if (!ne) return false;
  if (auto const cls = ne->clsList()) {
    if (!(cls->attrs() & AttrUnique)) return false;
    if (!cls->wasSerialized()) write_class(ser, cls);
    return true;
  }
  if (!ne->isPersistentTypeAlias()) return false;
  return write_type_alias(ser, ne->getCachedTypeAlias());
}

bool write_type_alias(ProfDataSerializer& ser, const TypeAliasReq* td) {
  SCOPE_EXIT {
    ITRACE(2, "TypeAlias: {}\n", td->name);
  };
  ITRACE(2, "TypeAlias>\n");
  // we're writing out Class* and TypeAliasReq* intermingled here. Set
  // bit 1 for TypeAliasReq's so we can distinguish when reading back
  // in. We also need to keep track of both whether we already tried
  // serialize this one, and whether it was successful. Use td2 for
  // that too.
  auto const td2 = reinterpret_cast<const char*>(td) + 2;
  if (!ser.serialize(td)) {
    return ser.wasSerialized(td2);
  }
  Trace::Indent _;

  auto const unit = td->unit;
  auto const name = td->name;
  auto const tas = unit->typeAliases();
  for (auto const& ta : tas) {
    if (ta.name == name) {
      switch (get_ts_kind(ta.typeStructure.get())) {
        case TypeStructure::Kind::T_unresolved: {
          auto const clsname = get_ts_classname(ta.typeStructure.get());
          if (!write_type_alias_or_class(ser,
                                         NamedEntity::get(clsname, false))) {
            return false;
          }
          break;
        }
        case TypeStructure::Kind::T_void:
        case TypeStructure::Kind::T_int:
        case TypeStructure::Kind::T_bool:
        case TypeStructure::Kind::T_float:
        case TypeStructure::Kind::T_string:
        case TypeStructure::Kind::T_resource:
        case TypeStructure::Kind::T_num:
        case TypeStructure::Kind::T_arraykey:
        case TypeStructure::Kind::T_noreturn:
        case TypeStructure::Kind::T_tuple:
        case TypeStructure::Kind::T_array:
        case TypeStructure::Kind::T_darray:
        case TypeStructure::Kind::T_varray:
        case TypeStructure::Kind::T_varray_or_darray:
        case TypeStructure::Kind::T_dict:
        case TypeStructure::Kind::T_vec:
        case TypeStructure::Kind::T_keyset:
        case TypeStructure::Kind::T_vec_or_dict:
        case TypeStructure::Kind::T_arraylike:
        case TypeStructure::Kind::T_nonnull:
          break;

        case TypeStructure::Kind::T_fun:
        case TypeStructure::Kind::T_typevar:
        case TypeStructure::Kind::T_shape:
        case TypeStructure::Kind::T_typeaccess:
        case TypeStructure::Kind::T_xhp:
        case TypeStructure::Kind::T_mixed:
          return false;

        case TypeStructure::Kind::T_class:
        case TypeStructure::Kind::T_interface:
        case TypeStructure::Kind::T_trait:
        case TypeStructure::Kind::T_enum:
          // these types don't occur in unresolved TypeStructures
          always_assert(false);
      }

      if (!ser.serialize(td2)) always_assert(false);
      write_serialized_ptr(ser, td2);
      write_unit(ser, td->unit);
      write_string(ser, td->name);
      return true;
    }
  }
  return false;
}

Class* read_class_internal(ProfDataDeserializer& ser) {
  auto const id = read_raw<decltype(std::declval<PreClass*>()->id())>(ser);
  auto const unit = read_unit(ser);

  read_container(ser,
                 [&] {
                   auto const dep = read_class(ser);
                   auto const ne = dep->preClass()->namedEntity();
                   // if it's not persistent, make sure that dep
                   // is the active class for this NamedEntity
                   assertx(ne->m_cachedClass.bound());
                   if (ne->m_cachedClass.isNormal()) {
                     ne->setCachedClass(dep);
                   }
                   auto const depName = read_string(ser);
                   if (!dep->name()->isame(depName)) {
                     // this dependent was referred to via a
                     // class_alias, so we need to make sure
                     // *that* points to the class too
                     auto const aliasNe = NamedEntity::get(depName);
                     aliasNe->m_cachedClass.bind(rds::Mode::Normal);
                     if (aliasNe->m_cachedClass.isNormal()) {
                       aliasNe->m_cachedClass.markUninit();
                     }
                     aliasNe->setCachedClass(dep);
                   }
                 });

  auto const preClass = unit->lookupPreClassId(id);
  folly::Optional<TypeAliasReq> enumBaseReq;
  SCOPE_EXIT {
    if (enumBaseReq) {
      preClass->enumBaseTy().namedEntity()->m_cachedTypeAlias.markUninit();
    }
  };
  if (preClass->attrs() & AttrEnum &&
      preClass->enumBaseTy().isObject()) {
    auto const dt = read_raw<DataType>(ser);
    auto const ne = preClass->enumBaseTy().namedEntity();
    if (!ne->m_cachedTypeAlias.bound() ||
        !ne->m_cachedTypeAlias.isInit()) {
      enumBaseReq.emplace();
      enumBaseReq->type = dt == KindOfInt64 ?
        AnnotType::Int : AnnotType::String;
      enumBaseReq->name = preClass->enumBaseTy().typeName();
      ne->m_cachedTypeAlias.bind(rds::Mode::Normal);
      ne->m_cachedTypeAlias.initWith(*enumBaseReq);
    }
  }
  auto const ne = preClass->namedEntity();
  // If it's not persistent, make sure its NamedEntity is
  // unbound, ready for DefClass
  if (ne->m_cachedClass.bound() &&
      ne->m_cachedClass.isNormal()) {
    ne->m_cachedClass.markUninit();
  }

  auto const cls = Unit::defClass(preClass, true);
  if (cls->pinitVec().size()) cls->initPropHandle();
  if (cls->numStaticProperties()) cls->initSPropHandles();

  if (cls->parent() == c_Closure::classof()) {
    auto const ctx = read_class(ser);
    if (ctx != cls) return cls->rescope(ctx);
  }
  return cls;
}

/*
 * This reads in TypeAliases and Classes that are used for code gen,
 * but otherwise aren't needed for profiling. We just need them to be
 * loaded into the NamedEntity table, so this function just returns
 * whether or not to continue (the end of the list is marked by a
 * null pointer).
 */
bool read_type_alias_or_class(ProfDataDeserializer& ser) {
  auto const ptr = read_raw<uintptr_t>(ser);
  if (!ptr) return false;
  assertx(ptr & 1);

  if (!(ptr & 2)) {
    ITRACE(2, "Class>\n");
    Trace::Indent _;
    auto& ent = ser.getEnt(reinterpret_cast<Class*>(ptr - 1));
    assertx(!ent);
    ent = read_class_internal(ser);
    assertx(ent);
    ITRACE(2, "Class: {}\n", ent->name());
    return true;
  }

  auto const unit = read_unit(ser);
  auto const name = read_string(ser);
  ITRACE(2, "TypeAlias: {}\n", name);
  auto const tas = unit->typeAliases();
  Id id = 0;
  for (auto const& ta : tas) {
    if (ta.name == name) {
      unit->defTypeAlias(id);
      return true;
    }
    ++id;
  }
  always_assert(false);
}

void write_profiled_funcs(ProfDataSerializer& ser, ProfData* pd) {
  auto const maxFuncId = pd->maxProfilingFuncId();

  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    if (!Func::isFuncIdValid(fid) || !pd->profiling(fid)) continue;
    write_func(ser, Func::fromFuncId(fid));
  }
  write_raw(ser, uintptr_t{});
}

void read_profiled_funcs(ProfDataDeserializer& ser, ProfData* pd) {
  while (auto const func = read_func(ser)) {
    pd->setProfiling(func->getFuncId());
  }
}

void write_classes_and_type_aliases(ProfDataSerializer& ser, ProfData* pd) {
  auto const maxFuncId = pd->maxProfilingFuncId();

  // in an attempt to get a sensible order for these, start with the
  // ones referenced by params and return constraints.
  for (FuncId fid = 0; fid <= maxFuncId; fid++) {
    if (!Func::isFuncIdValid(fid) || !pd->profiling(fid)) continue;
    auto const func = Func::fromFuncId(fid);
    for (auto const& p : func->params()) {
      write_type_alias_or_class(ser, p.typeConstraint.namedEntity());
    }
  }

  // Now just iterate and write anything that remains
  NamedEntity::foreach_name(
    [&] (NamedEntity& ne) {
      write_type_alias_or_class(ser, &ne);
    }
  );
  write_raw(ser, uintptr_t{});
}

void read_classes_and_type_aliases(ProfDataDeserializer& ser) {
  while (read_type_alias_or_class(ser)) {
    // nothing to do. this was just to make sure everything is loaded
    // into the NamedEntity table
  }
}

void write_prof_data(ProfDataSerializer& ser, ProfData* pd) {
  write_profiled_funcs(ser, pd);

  write_raw(ser, pd->counterDefault());
  pd->forEachTransRec(
    [&] (const ProfTransRec* ptr) {
      auto const transID = ptr->isProfile() ?
        ptr->region()->entry()->profTransID() :
        pd->proflogueTransId(ptr->func(), ptr->prologueArgs());
      write_raw(ser, transID);
      write_prof_trans_rec(ser, ptr, pd);
      // forEachTransRec already grabs a read lock, and we're not
      // going to add a *new* counter here (so we don't need a write
      // lock).
      write_raw(ser, *pd->transCounterAddrNoLock(transID));
    }
  );
  write_raw(ser, kInvalidTransID);
}

void read_prof_data(ProfDataDeserializer& ser, ProfData* pd) {
  read_profiled_funcs(ser, pd);

  pd->resetCounters(read_raw<int64_t>(ser));
  while (true) {
    auto const transID = read_raw<TransID>(ser);
    if (transID == kInvalidTransID) break;
    pd->addProfTrans(transID, read_prof_trans_rec(ser));
    *pd->transCounterAddr(transID) = read_raw<int64_t>(ser);
  }
}

template<typename T>
auto write_impl(ProfDataSerializer& ser, const T& out, bool) ->
  decltype(std::declval<T&>().serialize(ser),void()) {
  out.serialize(ser);
}

template<typename T>
void write_impl(ProfDataSerializer& ser, const T& out, int) {
  write_raw(ser, out);
}

template<typename T>
void write_maybe_serializable(ProfDataSerializer& ser, const T& out) {
  write_impl(ser, out, false);
}

struct TargetProfileVisitor : boost::static_visitor<void> {
  TargetProfileVisitor(ProfDataSerializer& ser,
                       const rds::Symbol& sym,
                       rds::Handle handle,
                       uint32_t size) :
      ser{ser},
      sym{sym},
      handle{handle},
      size{size} {}

  template<typename T>
  void process(T& out, const StringData* name) {
    write_raw(ser, size);
    write_string(ser, name);
    write_raw(ser, sym);
    TargetProfile<T>::reduce(out, handle, size);
    if (size == sizeof(T)) {
      write_maybe_serializable(ser, out);
    } else {
      write_raw(ser, &out, size);
    }
  }

  template<typename T> void operator()(const T&) {}
  template<typename T>
  void operator()(const rds::Profile<T>& pt) {
    if (size == sizeof(T)) {
      T out{};
      process(out, pt.name.get());
    } else {
      auto const mem = calloc(1, size);
      SCOPE_EXIT { free(mem); };
      process(*reinterpret_cast<T*>(mem), pt.name.get());
    }
  }

  ProfDataSerializer& ser;
  const rds::Symbol& sym;
  rds::Handle handle;
  uint32_t size;
};

void write_target_profiles(ProfDataSerializer& ser) {
  rds::visitSymbols(
    [&] (const rds::Symbol& symbol, rds::Handle handle, uint32_t size) {
      TargetProfileVisitor tv(ser, symbol, handle, size);
      boost::apply_visitor(tv, symbol);
    }
  );
  write_raw(ser, uint32_t{});
}

template<typename T>
auto read_impl(ProfDataDeserializer& ser, T& out, bool) ->
  decltype(out.deserialize(ser),void()) {
  out.deserialize(ser);
}

template<typename T>
void read_impl(ProfDataDeserializer& ser, T& out, int) {
  read_raw(ser, out);
}

template<typename T>
void read_maybe_serializable(ProfDataDeserializer& ser, T& out) {
  read_impl(ser, out, false);
}

struct SymbolFixup : boost::static_visitor<void> {
  SymbolFixup(ProfDataDeserializer& ser, StringData* name, uint32_t size) :
      ser{ser}, name{name}, size{size} {}

  template<typename T> void operator()(T&) { always_assert(false); }
  template<typename T>
  void operator()(rds::Profile<T>& pt) {
    TargetProfile<T> prof(pt.transId,
                          TransKind::Profile,
                          pt.bcOff,
                          name,
                          size - sizeof(T));

    if (size == sizeof(T)) {
      read_maybe_serializable(ser, prof.value());
    } else {
      read_raw(ser, &prof.value(), size);
    }
  }

  ProfDataDeserializer& ser;
  StringData* name;
  // The size of the original rds allocation.
  uint32_t size;
};

void read_target_profiles(ProfDataDeserializer& ser) {
  while (true) {
    auto const size = read_raw<uint32_t>(ser);
    if (!size) break;
    auto const name = read_string(ser);
    auto sym = read_raw<rds::Symbol>(ser);
    auto sf = SymbolFixup{ser, name, size};
    boost::apply_visitor(sf, sym);
  }
}

void merge_loaded_units(int numWorkers) {
  auto units = loadedUnitsRepoAuth();

  std::vector<std::thread> workers;
  // Compute a batch size that causes each thread to process approximately 16
  // batches.  Even if the batches are somewhat imbalanced in what they contain,
  // the straggler workers are very unlikey to take more than 10% longer than
  // the first worker to finish.
  auto const batchSize{std::max(units.size() / numWorkers / 16, size_t(1))};
  std::atomic<size_t> index{0};
  for (auto worker = 0; worker < numWorkers; ++worker) {
    workers.push_back(std::thread([&] {
      ProfileNonVMThread nonVM;
      hphp_thread_init();
      hphp_session_init(Treadmill::SessionKind::PreloadRepo);

      while (true) {
        auto begin = index.fetch_add(batchSize);
        auto end = std::min(begin + batchSize, units.size());
        if (begin >= end) break;
        auto unitCount = end - begin;
        for (auto i = size_t{0}; i < unitCount; ++i) {
          auto const unit = units[begin + i];
          try {
            unit->merge();
          } catch (...) {
            // swallow errors silently. persistent things should raise
            // errors, and we don't really care about merging
            // non-persistent things.
          }
        }
      }

      hphp_context_exit();
      hphp_session_exit();
      hphp_thread_exit();
    }));
  }
  for (auto& worker : workers) {
    worker.join();
  }
}

////////////////////////////////////////////////////////////////////////////////
}

ProfDataSerializer::ProfDataSerializer(const std::string& name)
  : fileName(name) {
  // Delete old profile data to avoid confusion.  This should've happened from
  // outside the process, but in case it didn't happen, try to do it here.
  unlink(name.c_str());

  std::string partialFile = name + ".part";
  fd = open(partialFile.c_str(),
            O_CLOEXEC | O_CREAT | O_TRUNC | O_WRONLY, 0644);
  if (fd == -1) {
    auto const msg =
      folly::sformat("Failed to open file for write {}, {}", name,
                     folly::errnoStr(errno));
    Logger::Error(msg);
    throw std::runtime_error(msg);
  }
}

void ProfDataSerializer::finalize() {
  assertx(fd != -1);
  if (offset) ::write(fd, buffer, offset);
  offset = 0;
  close(fd);
  fd = -1;
  std::string partialFile = fileName + ".part";
  if (rename(partialFile.c_str(), fileName.c_str()) == -1) {
    auto const msg =
      folly::sformat("Failed to rename {} to {}, {}",
                     partialFile, fileName, folly::errnoStr(errno));
    Logger::Error(msg);
    throw std::runtime_error(msg);
  } else {
    FTRACE(1, "Finished serializing profile data to " + fileName);
  }
}

ProfDataSerializer::~ProfDataSerializer() {
  if (fd != -1) {
    // We didn't finalize(), maybe because an exception was thrown while writing
    // the data.  The file is likely corrupt or incomplete, so discard it.
    ftruncate(fd, 0);
    close(fd);
    std::string partialFile = fileName + ".part";
    unlink(partialFile.c_str());
  }
}

ProfDataDeserializer::ProfDataDeserializer(const std::string& name) {
  fd = open(name.c_str(), O_CLOEXEC | O_RDONLY);
  if (fd == -1) throw std::runtime_error("Failed to open: " + name);
}

ProfDataDeserializer::~ProfDataDeserializer() {
  assertx(fd != -1);
  close(fd);
}

bool ProfDataDeserializer::done() {
  char byte;
  return offset == buffer_size && ::read(fd, &byte, 1) == 0;
}

void write_raw(ProfDataSerializer& ser, const void* data, size_t sz) {
  if (ser.offset + sz <= ProfDataSerializer::buffer_size) {
    memcpy(ser.buffer + ser.offset, data, sz);
    ser.offset += sz;
    return;
  }
  if (ser.offset == 0) {
    if (::write(ser.fd, data, sz) != sz) {
      throw std::runtime_error("Failed to write serialized data");
    }
    return;
  }
  if (auto const delta = ProfDataSerializer::buffer_size - ser.offset) {
    memcpy(ser.buffer + ser.offset, data, delta);
    data = static_cast<const char*>(data) + delta;
    sz -= delta;
    ser.offset = ProfDataSerializer::buffer_size;
  }
  assertx(ser.offset == ProfDataSerializer::buffer_size);
  if (::write(ser.fd, ser.buffer, ser.offset) != ser.offset) {
    throw std::runtime_error("Failed to write serialized data");
  }
  ser.offset = 0;
  write_raw(ser, data, sz);
}

void read_raw(ProfDataDeserializer& ser, void* data, size_t sz) {
  if (ser.offset + sz <= ProfDataDeserializer::buffer_size) {
    memcpy(data, ser.buffer + ser.offset, sz);
    ser.offset += sz;
    return;
  }
  if (auto const delta = ProfDataDeserializer::buffer_size - ser.offset) {
    memcpy(data, ser.buffer + ser.offset, delta);
    data = static_cast<char*>(data) + delta;
    sz -= delta;
    ser.offset = ProfDataDeserializer::buffer_size;
  }
  if (sz >= ProfDataDeserializer::buffer_size) {
    auto const bytes_read = ::read(ser.fd, data, sz);
    if (bytes_read < 0 || bytes_read < sz) {
      throw std::runtime_error("Failed to read serialized data");
    }
    return;
  }

  auto const bytes_read = ::read(ser.fd,
                                 ser.buffer,
                                 ProfDataDeserializer::buffer_size);
  if (bytes_read < 0 || bytes_read < sz) {
    throw std::runtime_error("Failed to read serialized data");
  }
  ser.offset = ProfDataDeserializer::buffer_size - bytes_read;
  if (ser.offset) {
    memmove(ser.buffer + ser.offset, ser.buffer, bytes_read);
  }
  return read_raw(ser, data, sz);
}

StringData*& ProfDataDeserializer::getEnt(const StringData* p) {
  return stringMap[uintptr_t(p)];
}

ArrayData*& ProfDataDeserializer::getEnt(const ArrayData* p) {
  return arrayMap[uintptr_t(p)];
}

Unit*& ProfDataDeserializer::getEnt(const Unit* p) {
  return unitMap[uintptr_t(p)];
}

Func*& ProfDataDeserializer::getEnt(const Func* p) {
  return funcMap[uintptr_t(p)];
}

Class*& ProfDataDeserializer::getEnt(const Class* p) {
  return classMap[uintptr_t(p)];
}

const RepoAuthType::Array*&
ProfDataDeserializer::getEnt(const RepoAuthType::Array* p) {
  return ratMap[uintptr_t(p)];
}

bool ProfDataSerializer::serialize(const Unit* unit) {
  return unit->serialize();
}

bool ProfDataSerializer::serialize(const Func* func) {
  return func->serialize();
}

bool ProfDataSerializer::serialize(const Class* cls) {
  return cls->serialize();
}

void write_string(ProfDataSerializer& ser, const StringData* str) {
  if (!ser.serialize(str)) return write_raw(ser, str);
  write_serialized_ptr(ser, str);
  uint32_t sz = str->size();
  write_raw(ser, sz);
  write_raw(ser, str->data(), sz);
}

StringData* read_string(ProfDataDeserializer& ser) {
  return deserialize(
    ser,
    [&] () -> StringData* {
      auto const sz = read_raw<uint32_t>(ser);
      String s{sz, ReserveStringMode{}};
      auto const range = s.bufferSlice();
      read_raw(ser, range.begin(), sz);
      s.setSize(sz);
      return makeStaticString(s);
    }
  );
}

void write_array(ProfDataSerializer& ser, const ArrayData* arr) {
  if (!ser.serialize(arr)) return write_raw(ser, arr);
  write_serialized_ptr(ser, arr);
  auto const str = internal_serialize(VarNR(const_cast<ArrayData*>(arr)));
  uint32_t sz = str.size();
  write_raw(ser, sz);
  write_raw(ser, str.data(), sz);
}

ArrayData* read_array(ProfDataDeserializer& ser) {
  return deserialize(
    ser,
    [&] () -> ArrayData* {
      auto const sz = read_raw<uint32_t>(ser);
      String s{sz, ReserveStringMode{}};
      auto const range = s.bufferSlice();
      read_raw(ser, range.begin(), sz);
      s.setSize(sz);
      auto v = unserialize_from_buffer(
        s.data(),
        s.size(),
        VariableUnserializer::Type::Internal
      );
      return ArrayData::GetScalarArray(std::move(v));
    }
  );
}

void write_unit(ProfDataSerializer& ser, const Unit* unit) {
  if (!ser.serialize(unit)) return write_raw(ser, unit);
  ITRACE(2, "Unit: {}\n", unit->filepath());
  write_serialized_ptr(ser, unit);
  write_string(ser, unit->filepath());
}

Unit* read_unit(ProfDataDeserializer& ser) {
  return deserialize(
    ser,
    [&] () -> Unit* {
      auto const filepath = read_string(ser);
      ITRACE(2, "Unit: {}\n", filepath);
      if (filepath->data()[0] == '/' && filepath->data()[1] == ':') {
        return lookupSyslibUnit(filepath);
      }
      return lookupUnit(filepath, "", nullptr);
    }
  );
}

template<typename C1, typename C2, typename F>
void visit_deps(const C1& c1, const C2& c2, F& f) {
  auto it = c2.begin();
  auto const DEBUG_ONLY end = c2.end();
  for (auto const& dep : c1) {
    assertx(it != end);
    f(dep.get(), *it++);
  }
}

void write_class(ProfDataSerializer& ser, const Class* cls) {
  SCOPE_EXIT {
    ITRACE(2, "Class: {}\n", cls ? cls->name() : staticEmptyString());
  };
  ITRACE(2, "Class>\n");
  Trace::Indent _;

  if (!cls || !ser.serialize(cls)) return write_raw(ser, cls);

  write_serialized_ptr(ser, cls);
  write_raw(ser, cls->preClass()->id());
  write_unit(ser, cls->preClass()->unit());

  jit::vector<std::pair<const Class*, const StringData*>> dependents;
  auto record_dep = [&] (const Class* dep, const StringData* depName) {
    if (!dep) return;
    if (!dep->wasSerialized() ||
        !classHasPersistentRDS(dep) ||
        !dep->name()->isame(depName)) {
      dependents.emplace_back(dep, depName);
    }
  };
  record_dep(cls->parent(), cls->preClass()->parent());

  visit_deps(cls->declInterfaces(), cls->preClass()->interfaces(), record_dep);

  if (cls->preClass()->attrs() & AttrNoExpandTrait) {
    for (auto const tName : cls->preClass()->usedTraits()) {
      auto const trait = Unit::lookupUniqueClassInContext(tName, nullptr);
      assertx(trait);
      record_dep(trait, tName);
    }
  } else {
    visit_deps(cls->usedTraitClasses(),
               cls->preClass()->usedTraits(),
               record_dep);
  }

  write_container(ser, dependents,
                  [&] (const std::pair<const Class*, const StringData*> &dep) {
                    write_class(ser, dep.first);
                    write_string(ser, dep.second);
                  });

  if (cls->attrs() & AttrEnum &&
      cls->preClass()->enumBaseTy().isObject()) {
    if (cls->enumBaseTy()) {
      write_raw(ser, *cls->enumBaseTy());
    } else {
      write_raw(ser, KindOfUninit);
    }
  }

  if (cls->parent() == c_Closure::classof()) {
    auto const func = cls->lookupMethod(s_invoke.get());
    assertx(func);
    write_class(ser, func->cls());
  }
}

Class* read_class(ProfDataDeserializer& ser) {
  ITRACE(2, "Class>\n");
  auto const ret = deserialize(
    ser,
    [&] () -> Class* {
      Trace::Indent _;
      return read_class_internal(ser);
    }
  );

  ITRACE(2, "Class: {}\n", ret ? ret->name() : staticEmptyString());
  return ret;
}

void write_func(ProfDataSerializer& ser, const Func* func) {
  SCOPE_EXIT {
    ITRACE(2, "Func: {}\n", func ? func->fullName() : staticEmptyString());
  };
  ITRACE(2, "Func>\n");
  Trace::Indent _;

  if (!func || !ser.serialize(func)) return write_raw(ser, func);

  write_serialized_ptr(ser, func);
  uint32_t fid = func->getFuncId();
  assertx(!(fid & 0x80000000));
  if (func == SystemLib::s_nullCtor ||
      (!func->isMethod() && !func->isPseudoMain() && func->isBuiltin())) {
    if (func == SystemLib::s_nullCtor) {
      assertx(func->name()->isame(s_86ctor.get()));
    }
    fid = ~fid;
    write_raw(ser, fid);
    return write_string(ser, func->name());
  }
  write_raw(ser, fid);
  if (func->isPseudoMain()) {
    const uint32_t zero = 0;
    write_raw(ser, zero);
    write_unit(ser, func->unit());
    return write_class(ser, func->cls());
  }

  if (func->isMethod()) {
    auto const* cls = func->implCls();
    auto const nslot = [&] () -> uint32_t {
      const uint32_t slot = func->methodSlot();
      if (cls->getMethod(slot) != func) {
        if (func->name() == s_86pinit.get()) return k86pinitSlot;
        if (func->name() == s_86sinit.get()) return k86sinitSlot;
        if (func->name() == s_86linit.get()) return k86linitSlot;
        cls = getOwningClassForFunc(func);
        assertx(cls->getMethod(slot) == func);
      }
      return ~slot;
    }();
    assertx(nslot & 0x80000000);
    write_raw(ser, nslot);
    return write_class(ser, cls);
  }

  // Ideally we'd write the func's index in its Unit; but we may not
  // have that after Unit::initial_merge
  const uint32_t off = func->base();
  assertx(off && !(off & 0x80000000));
  write_raw(ser, off);
  write_unit(ser, func->unit());
}

Func* read_func(ProfDataDeserializer& ser) {
  ITRACE(2, "Func>\n");
  auto const ret = deserialize(
    ser,
    [&] () -> Func* {
      Trace::Indent _;
      auto fid = read_raw<uint32_t>(ser);
      auto const func = [&] () -> const Func* {
        if (fid & 0x80000000) {
          fid = ~fid;
          auto const name = read_string(ser);
          if (name->isame(s_86ctor.get())) return SystemLib::s_nullCtor;
          return Unit::lookupFunc(name);
        }
        auto const id = read_raw<uint32_t>(ser);
        if (!id) {
          auto const unit = read_unit(ser);
          return unit->getMain(read_class(ser));
        }
        if (id & 0x80000000) {
          auto const cls = read_class(ser);
          if (id == k86pinitSlot) return cls->get86pinit();
          if (id == k86sinitSlot) return cls->get86sinit();
          if (id == k86linitSlot) return cls->get86linit();
          const Slot slot = ~id;
          return cls->getMethod(slot);
        }
        auto const unit = read_unit(ser);
        for (auto const f : unit->funcs()) {
          if (f->base() == id) {
            Unit::bindFunc(f);
            auto const handle = f->funcHandle();
            if (!rds::isPersistentHandle(handle) &&
                (!rds::isHandleInit(handle, rds::NormalTag{}) ||
                 rds::handleToRef<LowPtr<Func>,
                                  rds::Mode::Normal>(handle).get() != f)) {
              rds::uninitHandle(handle);
              Unit::defFunc(f, false);
            }
            return f;
          }
        }
        not_reached();
      }();
      ser.recordFid(fid, func->getFuncId());
      return const_cast<Func*>(func);
    }
  );
  ITRACE(2, "Func: {}\n", ret ? ret->fullName() : staticEmptyString());
  return ret;
}

std::string serializeProfData(const std::string& filename) {
  try {
    ProfDataSerializer ser{filename};

    write_raw(ser, Repo::get().global().Signature);
    auto schema = repoSchemaId();
    write_raw(ser, schema.size());
    write_raw(ser, schema.begin(), schema.size());

    Func::s_treadmill = true;
    hphp_thread_init();
    hphp_session_init(Treadmill::SessionKind::ProfData);
    requestInitProfData();

    SCOPE_EXIT {
      requestExitProfData();
      hphp_context_exit();
      hphp_session_exit();
      hphp_thread_exit();
      Func::s_treadmill = false;
    };

    InstanceBits::init();
    InstanceBits::serialize(ser);
    write_global_array_map(ser);

    auto const pd = profData();
    write_prof_data(ser, pd);

    write_target_profiles(ser);

    // We've written everything directly referenced by the profile
    // data, but jitted code might still use Classes and TypeAliasReqs
    // that haven't been otherwise mentioned (eg VerifyParamType,
    // InstanceOfD etc).
    write_classes_and_type_aliases(ser, pd);

    ser.finalize();

    return "";
  } catch (std::runtime_error& err) {
    return folly::sformat("Failed to serialize profile data: {}", err.what());
  }
}

std::string deserializeProfData(const std::string& filename, int numWorkers) {
  try {
    ProfDataDeserializer ser{filename};

    auto signature = read_raw<decltype(Repo::get().global().Signature)>(ser);
    if (signature != Repo::get().global().Signature) {
      throw std::runtime_error("Mismatched repo-schema");
    }
    auto size = read_raw<size_t>(ser);
    std::string schema;
    schema.resize(size);
    read_raw(ser, &schema[0], size);
    if (schema != repoSchemaId()) {
      throw std::runtime_error("Mismatched repo-schema");
    }

    InstanceBits::deserialize(ser);
    read_global_array_map(ser);

    ProfData::Session pds;
    auto const pd = profData();
    read_prof_data(ser, pd);
    pd->setDeserialized();

    read_target_profiles(ser);

    read_classes_and_type_aliases(ser);

    always_assert(ser.done());

    // During deserialization we didn't merge the loaded units because
    // we wanted to pick and choose the hot Funcs and Classes. But we
    // need to merge them before we start serving traffic to ensure we
    // don't have inconsistentcies (eg a persistent memoized Func
    // wrapper might have been merged, while its implementation was
    // not; since the implementation has an internal name, there won't
    // be an autoload entry for it, so unless something else causes
    // the unit to be loaded the implementation might never get pulled
    // in (resulting in fatals when the wrapper tries to call it).
    merge_loaded_units(numWorkers);

    return "";
  } catch (std::runtime_error& err) {
    return folly::sformat("Failed to deserialize profile data {}: {}",
                          filename, err.what());
  }
}

//////////////////////////////////////////////////////////////////////
} }
