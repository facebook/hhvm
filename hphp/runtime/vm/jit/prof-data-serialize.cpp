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
#include "hphp/runtime/base/configs/jit.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/vm-worker.h"

#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/ext/core/ext_core_closure.h"

#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/instance-bits.h"
#include "hphp/runtime/vm/jit/array-access-profile.h"
#include "hphp/runtime/vm/jit/array-iter-profile.h"
#include "hphp/runtime/vm/jit/array-layout.h"
#include "hphp/runtime/vm/jit/call-target-profile.h"
#include "hphp/runtime/vm/jit/cls-cns-profile.h"
#include "hphp/runtime/vm/jit/coeffect-fun-param-profile.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/cow-profile.h"
#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/func-order.h"
#include "hphp/runtime/vm/jit/incref-profile.h"
#include "hphp/runtime/vm/jit/inlining-decider.h"
#include "hphp/runtime/vm/jit/meth-profile.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/switch-profile.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc-record.h"
#include "hphp/runtime/vm/jit/trans-cfg.h"
#include "hphp/runtime/vm/jit/type-profile.h"
#include "hphp/runtime/vm/jit/vasm-block-counters.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/property-profile.h"
#include "hphp/runtime/vm/repo-file.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/type-profile.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/boot-stats.h"
#include "hphp/util/build-info.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#include "hphp/util/managed-arena.h"
#include "hphp/util/match.h"
#include "hphp/util/numa.h"
#include "hphp/util/process.h"
#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"

#include <folly/portability/Unistd.h>
#include <folly/String.h>

namespace HPHP { namespace jit {
//////////////////////////////////////////////////////////////////////

namespace {
//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbc);

StaticString s_invoke("__invoke");

constexpr uint32_t kMagic = 0x4d564848;

constexpr uint32_t k86pinitSlot = 0x80000000u;
constexpr uint32_t k86sinitSlot = 0x80000001u;
constexpr uint32_t k86linitSlot = 0x80000002u;

Optional<std::string> s_deserializedFile{};

enum class SeenType : uint8_t {
  Class,
  TypeAlias,
  End
};

constexpr ProfDataSerializer::Id kSerializedIdBit =
  (ProfDataSerializer::Id{1} <<
   (std::numeric_limits<ProfDataSerializer::Id>::digits - 1));

ProfDataSerializer::Id read_id(ProfDataDeserializer& ser) {
  auto const id = read_raw<ProfDataSerializer::Id>(ser);
  always_assert(!(id & kSerializedIdBit));
  return id;
}

Unit* deserializeImpl(ProfDataDeserializer& ser,
                      ProfDataSerializer::Id id,
                      Unit*) {
  return ser.getUnit(id);
}
Func* deserializeImpl(ProfDataDeserializer& ser,
                      ProfDataSerializer::Id id,
                      Func*) {
  return ser.getFunc(id);
}
Class* deserializeImpl(ProfDataDeserializer& ser,
                       ProfDataSerializer::Id id,
                       Class*) {
  return ser.getClass(id);
}
const TypeAlias* deserializeImpl(ProfDataDeserializer& ser,
                                 ProfDataSerializer::Id id,
                                 const TypeAlias*) {
  return ser.getTypeAlias(id);
}
ArrayData* deserializeImpl(ProfDataDeserializer& ser,
                           ProfDataSerializer::Id id,
                           ArrayData*) {
  return ser.getArray(id);
}
StringData* deserializeImpl(ProfDataDeserializer& ser,
                            ProfDataSerializer::Id id,
                            StringData*) {
  return ser.getString(id);
}
const RepoAuthType::Array* deserializeImpl(ProfDataDeserializer& ser,
                                           ProfDataSerializer::Id id,
                                           const RepoAuthType::Array*) {
  return ser.getArrayRAT(id);
}
FuncId deserializeImpl(ProfDataDeserializer& ser,
                       ProfDataSerializer::Id id,
                       FuncId) {
  return ser.getFuncId(id);
}

template<typename F>
auto deserialize(ProfDataDeserializer& ser, F&& f) -> decltype(f()) {
  using T = decltype(f());
  auto const id = read_raw<ProfDataSerializer::Id>(ser);
  if (id & kSerializedIdBit) {
    auto const ent = f();
    ser.record(id - kSerializedIdBit, ent);
    return ent;
  }
  return deserializeImpl(ser, id, T{});
}

void write_serialized_id(ProfDataSerializer& ser, ProfDataSerializer::Id id) {
  assertx(!(id & kSerializedIdBit));
  write_raw(ser, id | kSerializedIdBit);
}

void write_id(ProfDataSerializer& ser, ProfDataSerializer::Id id) {
  assertx(!(id & kSerializedIdBit));
  write_raw(ser, id);
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

void write_unit_preload(ProfDataSerializer& ser, Unit* unit) {
  write_raw_string(ser, unit->origFilepath());
}

struct UnitPreloader : JobQueueWorker<StringData*, void*> {
  virtual void onThreadEnter() override {
    rl_typeProfileLocals->nonVMThread = true;
    hphp_session_init(Treadmill::SessionKind::PreloadRepo);
  }
  virtual void onThreadExit() override {
    hphp_context_exit();
    hphp_session_exit();
  }
  virtual void doJob(StringData* path) override {
    DEBUG_ONLY auto unit = lookupUnit(path, "", nullptr, nullptr, false);
    FTRACE(2, "Preloaded unit with path {}\n", path->data());
    assertx(unit->origFilepath() == path);  // both static
  }
};
using UnitPreloadDispatcher = JobQueueDispatcher<UnitPreloader>;
UnitPreloadDispatcher* s_preload_dispatcher;

void read_unit_preload(ProfDataDeserializer& ser) {
  auto const path =
    read_raw_string(ser, /* skip = */ !Cfg::Jit::DesUnitPreload);
  // path may be nullptr when JitDesUnitPreload isn't set.
  if (Cfg::Jit::DesUnitPreload) {
    assertx(path);
    s_preload_dispatcher->enqueue(path);
  }
}

void write_units_preload(ProfDataSerializer& ser) {
  std::vector<Unit*> units;
  hphp_fast_set<Unit*, pointer_hash<Unit>> seen;
  auto const check_unit =
    [] (Unit* unit) -> bool {
      if (!unit) return false;
      auto const filepath = unit->origFilepath();
      if (filepath->empty()) return false;
      // skip systemlib
      if (filepath->size() >= 2 && filepath->data()[1] == ':') return false;
      return true;
    };
  auto const pd = profData();
  assertx(pd);
  pd->forEachProfilingFunc([&](auto const& func) {
    always_assert(func);
    auto const u = func->unit();
    if (!check_unit(u)) return;
    if (seen.insert(u).second) units.push_back(u);
  });
  auto all_loaded = loadedUnitsRepoAuth();
  for (auto u : all_loaded) {
    if (!check_unit(u)) continue;
    if (seen.insert(u).second) units.push_back(u);
  }
  write_container(ser, units, write_unit_preload);
}

void read_units_preload(ProfDataDeserializer& ser) {
  BootStats::Block timer("DES_read_units_preload",
                         RuntimeOption::ServerExecutionMode());
  if (Cfg::Jit::DesUnitPreload) {
    auto const threads =
      std::max(Cfg::Jit::WorkerThreadsForSerdes, 1);
    s_preload_dispatcher = new UnitPreloadDispatcher(
        threads, threads, 0, false, nullptr
    );
    s_preload_dispatcher->start();
  }
  read_container(ser, [&] { read_unit_preload(ser); });
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

void write_region_block(ProfDataSerializer& ser,
                        const RegionDesc::BlockPtr& block) {
  write_raw(ser, block->id());
  write_srckey(ser, block->start());
  write_raw(ser, block->length());
  write_raw(ser, block->initialSpOffset());
  write_raw(ser, block->profTransID());
  write_container(ser, block->typePreConditions(), write_guarded_location);
  write_container(ser, block->postConds().changed, write_typed_location);
  write_container(ser, block->postConds().refined, write_typed_location);
}

RegionDesc::BlockPtr read_region_block(ProfDataDeserializer& ser) {
  auto const id = read_raw<RegionDesc::BlockId>(ser);
  auto const start = read_srckey(ser);
  auto const length = read_raw<int>(ser);
  auto const initialSpOffset = read_raw<SBInvOffset>(ser);

  auto const block = std::make_shared<RegionDesc::Block>(
    id, start, length, initialSpOffset);

  block->setProfTransID(read_raw<TransID>(ser));

  read_container(ser,
                 [&] {
                   block->addPreCondition(read_guarded_location(ser));
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
    write_srckey(ser, ptr->lastSrcKey());
    write_region_desc(ser, ptr->region().get());
    write_raw(ser, ptr->asmSize());
  } else {
    write_raw(ser, ptr->prologueArgs());

    auto lock = ptr->lockCallerList();
    std::vector<TransID> callers;
    auto addCaller = [&] (TCA caller) {
      auto const callerTransId = pd->jmpTransID(caller);
      if (callerTransId != kInvalidTransID) {
        callers.push_back(callerTransId);
      }
    };
    for (auto const caller : ptr->mainCallers()) addCaller(caller);
    write_container(ser, callers, write_raw<TransID>);
    write_raw(ser, ptr->asmSize());
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
    auto const lastSk = read_srckey(ser);
    auto const region = read_region_desc(ser);
    auto const asmSize = read_raw<uint32_t>(ser);
    return std::make_unique<ProfTransRec>(lastSk, sk, region, asmSize);
  }

  auto ret = std::make_unique<ProfTransRec>(sk, read_raw<int>(ser), 0);

  read_container(ser,
                 [&] {
                   ret->profCallers().push_back(read_raw<TransID>(ser));
                 });
  auto const asmSize = read_raw<uint32_t>(ser);
  ret->setAsmSize(asmSize);
  return ret;
}

bool write_seen_type(ProfDataSerializer& ser, const NamedType* ne) {
  if (!ne) return false;
  if (auto const cls = ne->clsList()) {
    if (!(cls->attrs() & AttrPersistent)) return false;
    auto const filepath = cls->preClass()->unit()->origFilepath();
    if (!filepath || filepath->empty()) return false;
    if (!ser.present(cls)) {
      write_raw(ser, SeenType::Class);
      write_class(ser, cls);
    }
    return true;
  } else if (auto const td = ne->getCachedTypeAlias()) {
    if (!ne->isPersistentTypeAlias()) return false;
    auto const filepath = td->unit()->origFilepath();
    if (!filepath || filepath->empty()) return false;
    if (!ser.present(td)) {
      write_raw(ser, SeenType::TypeAlias);
      write_typealias(ser, td);
    }
    return true;
  }
  return false;
}

void write_named_type(ProfDataSerializer& ser, const NamedType* ne) {
  always_assert(ne);
  if (auto const cls = ne->clsList()) {
    write_raw(ser, SeenType::Class);
    write_class(ser, cls);
  } else if (auto const td = ne->getCachedTypeAlias()) {
    write_raw(ser, SeenType::TypeAlias);
    write_typealias(ser, td);
  } else {
    always_assert(false);
  }
}

void read_named_type(ProfDataDeserializer& ser);

Class* read_class_internal(ProfDataDeserializer& ser) {
  auto const name = read_string(ser);
  auto const unit = read_unit(ser);

  read_container(ser,
                 [&] {
                   auto const dep = read_class(ser);
                   auto const ne = dep->preClass()->namedType();
                   // if it's not persistent, make sure that dep
                   // is the active class for this NamedType
                   assertx(ne->m_cachedClass.bound());
                   if (ne->m_cachedClass.isNormal()) {
                     ne->setCachedClass(dep);
                   }
                 });

  auto const preClass = unit->lookupPreClass(name);
  assertx(preClass);

  if (preClass->attrs() & AttrEnum &&
      preClass->enumBaseTy().isUnresolved()) {
    read_named_type(ser);
  }

  auto const ne = preClass->namedType();
  // If it's not persistent, make sure its NamedType is
  // unbound, ready for DefClass
  if (ne->m_cachedClass.bound() &&
      ne->m_cachedClass.isNormal()) {
    ne->m_cachedClass.markUninit();
  }

  if (preClass->parent() == c_Closure::classof()->name()) {
    auto const ctx = read_class(ser);
    auto const cls = Class::defClosure(preClass, true);
    assertx(!cls->needInitialization());
    return cls->rescope(ctx);
  } else {
    auto const cls = Class::def(preClass, true);
    if (cls->pinitVec().size()) cls->initPropHandle();
    if (cls->numStaticProperties()) cls->initSPropHandles();
    return cls;
  }
}

const TypeAlias* read_typealias_internal(ProfDataDeserializer& ser) {
  const Id id = read_raw<Id>(ser);
  auto const unit = read_unit(ser);
  read_raw<TypeConstraint>(ser);
  auto const td = unit->lookupTypeAliasId(id);
  return TypeAlias::def(td);
}

/*
 * This reads in TypeAliases and Classes that are used for code gen,
 * but otherwise aren't needed for profiling. We just need them to be
 * loaded into the NamedType table, so this function just returns
 * whether or not to continue.
 */
bool read_seen_type(ProfDataDeserializer& ser) {
  auto const type = read_raw<SeenType>(ser);
  switch (type) {
    case SeenType::Class: {
      read_class(ser);
      return true;
    }
    case SeenType::TypeAlias: {
      read_typealias(ser);
      return true;
    }
    case SeenType::End: return false;
    default:
      always_assert(false);
  }
}

void read_named_type(ProfDataDeserializer& ser) {
  auto const type = read_raw<SeenType>(ser);
  switch (type) {
    case SeenType::Class:
      read_class(ser);
      return;
    case SeenType::TypeAlias:
      read_typealias(ser);
      return;
    default:
      always_assert(false);
  }
}

void write_profiled_funcs(ProfDataSerializer& ser, ProfData* pd) {
  pd->forEachProfilingFunc([&](auto const& func) {
    always_assert(func);
    write_func(ser, func);
  });
  write_func(ser, nullptr);
}

void read_profiled_funcs(ProfDataDeserializer& ser, ProfData* pd) {
  while (auto const func = read_func(ser)) {
    pd->setProfiling(func);
  }
}

void write_seen_types(ProfDataSerializer& ser, ProfData* pd) {
  // in an attempt to get a sensible order for these, start with the
  // ones referenced by params and return constraints.
  pd->forEachProfilingFunc([&](auto const& func) {
    always_assert(func);
    for (auto const& p : func->params()) {
      if (auto const ne = p.typeConstraint.anyNamedType()) {
        write_seen_type(ser, ne);
      }
    }
  });

  // Now just iterate and write anything that remains
  NamedType::foreach_name(
    [&] (NamedType& ne) {
      write_seen_type(ser, &ne);
    }
  );
  write_raw(ser, SeenType::End);
}

void read_seen_types(ProfDataDeserializer& ser) {
  BootStats::Block timer("DES_read_classes_and_type_aliases",
                         RuntimeOption::ServerExecutionMode());
  while (read_seen_type(ser)) {
    // nothing to do. this was just to make sure everything is loaded
    // into the NamedType table
  }
}

void write_prof_data(ProfDataSerializer& ser, ProfData* pd) {
  // Write the profiled metadata to output
  std::string metaFile = ser.filename() + ".meta";
  if (auto out = fopen(metaFile.c_str(), "w")) {
    fprintf(out, "profFuncCnt=%ld\n", pd->profilingFuncs());
    fprintf(out, "profBCSize=%ld\n", pd->profilingBCSize());
    fclose(out);
  }

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
  write_raw<uint64_t>(ser, pd->baseProfCount());
}

void maybe_output_prof_trans_rec_trace(
  TransID transId, const ProfTransRec* profTransRec, uint64_t translationWeight) {
  if (profTransRec->kind() != TransKind::Profile) {
    return;
  }
  if (HPHP::Trace::moduleEnabledRelease(HPHP::Trace::print_profiles)) {
    auto const sk = profTransRec->srcKey();
    auto const unit = sk.unit();
    auto const func = sk.func();
    const char *filePath = "";
    if (func->originalFilename() && func->originalFilename()->size()) {
      filePath = func->originalFilename()->data();
    } else if (unit->origFilepath()->data() && unit->origFilepath()->size()) {
      filePath = unit->origFilepath()->data();
    }
    folly::dynamic blocks = folly::dynamic::array;
    for (const auto& block : profTransRec->region()->blocks()) {
      const auto typePreconditionsStr = show(block->typePreConditions());
      if (!typePreconditionsStr.empty()) {
        blocks.push_back(folly::dynamic::object("type_preconditions_raw", typePreconditionsStr));
      }
    }
    folly::dynamic profTransRecProfile = folly::dynamic::object;
    profTransRecProfile["end_bytecode_offset"] = profTransRec->lastSrcKey().printableOffset();
    profTransRecProfile["end_line_number"] = profTransRec->lastSrcKey().lineNumber();
    profTransRecProfile["file_path"] = filePath;
    profTransRecProfile["function_name"] = sk.func()->fullName()->data();
    profTransRecProfile["profile"] = folly::dynamic::object("profileType", "ProfTransRec");
    profTransRecProfile["region"] = folly::dynamic::object("blocks", blocks);
    profTransRecProfile["start_bytecode_offset"] = profTransRec->srcKey().printableOffset();
    profTransRecProfile["start_line_number"] = profTransRec->region()->start().lineNumber();
    profTransRecProfile["translation_weight"] = translationWeight;
    HPHP::Trace::traceRelease("json:%s\n", folly::toJson(profTransRecProfile).c_str());
  }
}

void read_prof_data(ProfDataDeserializer& ser, ProfData* pd) {
  BootStats::Block timer("DES_read_prof_data",
                         RuntimeOption::ServerExecutionMode());
  read_profiled_funcs(ser, pd);

  pd->resetCounters(read_raw<int64_t>(ser));
  while (true) {
    auto const transID = read_raw<TransID>(ser);
    if (transID == kInvalidTransID) break;
    pd->addProfTrans(transID, read_prof_trans_rec(ser));
    *pd->transCounterAddr(transID) = read_raw<int64_t>(ser);
    auto const profTransRec = pd->transRec(transID);
    maybe_output_prof_trans_rec_trace(transID, profTransRec, pd->transCounter(transID));
  }
  pd->setBaseProfCount(read_raw<uint64_t>(ser));
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
                       uint32_t size)
    : ser{ser}
    , sym{sym}
    , handle{handle}
    , size{size}
  {}

  template<typename T>
  void process(T& out, const StringData* name) {
    write_raw(ser, size);
    write_string(ser, name);
    auto& p = boost::get<rds::Profile>(sym);
    write_raw(ser, p.kind);
    write_raw(ser, p.transId);
    write_raw(ser, p.bcOff);
    write_string(ser, p.name);
    TargetProfile<T>::reduce(out, handle, size);
    write_maybe_serializable(ser, out);
  }

  template<typename T> void operator()(const T&) {}

  template<typename T>
  void go(const rds::Profile& pt) {
    if (size == sizeof(T)) {
      T out{};
      process(out, pt.name.get());
    } else {
      auto const mem = calloc(1, size);
      SCOPE_EXIT { free(mem); };
      process(*reinterpret_cast<T*>(mem), pt.name.get());
    }
  }

  void operator()(const rds::Profile& pt) {
    switch (pt.kind) {
      case rds::ProfileKind::None: always_assert(false);
#define PR(T)                   \
      case rds::ProfileKind::T: \
        return go<T>(pt);
      RDS_PROFILE_SYMBOLS
#undef PR
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

void write_rds_ordering_item(ProfDataSerializer& ser,
                             const rds::Ordering::Item& item) {
  write_string(ser, item.key);
  write_raw(ser, item.size);
  write_raw(ser, item.alignment);
}

rds::Ordering::Item read_rds_ordering_item(ProfDataDeserializer& des) {
  auto const key = read_cpp_string(des);
  auto const size =
    read_raw<decltype(rds::Ordering::Item::size)>(des);
  auto const alignment =
    read_raw<decltype(rds::Ordering::Item::alignment)>(des);
  return rds::Ordering::Item{key, size, alignment};
}

void write_rds_ordering(ProfDataSerializer& ser) {
  rds::Ordering order;
  if (RO::EvalReorderRDS) order = rds::profiledOrdering();
  write_container(ser, order.persistent, write_rds_ordering_item);
  write_container(ser, order.local, write_rds_ordering_item);
  write_container(ser, order.normal, write_rds_ordering_item);
}

rds::Ordering read_rds_ordering(ProfDataDeserializer& des) {
  rds::Ordering order;
  read_container(
    des,
    [&] { order.persistent.emplace_back(read_rds_ordering_item(des)); }
  );
  read_container(
    des,
    [&] { order.local.emplace_back(read_rds_ordering_item(des)); }
  );
  read_container(
    des,
    [&] { order.normal.emplace_back(read_rds_ordering_item(des)); }
  );
  return order;
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

template<typename T>
void maybe_output_target_profile_trace(
  const StringData* name, const TargetProfile<T>& prof, const rds::Profile &pt) {
  if (HPHP::Trace::moduleEnabledRelease(HPHP::Trace::print_profiles, 1)) {
    auto const pd = profData();
    assertx(pd != nullptr);
    if (isValidTransID(pt.transId)) {
      auto const ptr = pd->transRec(pt.transId);
      if (ptr) {
        auto const srcKey = ptr->srcKey();
        auto const func = srcKey.func();
        auto const unit = srcKey.unit();
        const char *filePath = "";
        if (func->originalFilename() && func->originalFilename()->size()) {
          filePath = func->originalFilename()->data();
        } else if (unit->origFilepath()->data() && unit->origFilepath()->size()) {
          filePath = unit->origFilepath()->data();
        }
        folly::dynamic targetProfileInfo = folly::dynamic::object;
        targetProfileInfo["trans_id"] = pt.transId;
        targetProfileInfo["profile_raw_name"] = name->toCppString();
        targetProfileInfo["profile"] = prof.value().toDynamic();
        targetProfileInfo["file_path"] = filePath;
        targetProfileInfo["line_number"] = func->getLineNumber(pt.bcOff);
        targetProfileInfo["function_name"] = func->fullName()->data();
        targetProfileInfo["start_bytecode_offset"] = pt.bcOff;
        HPHP::Trace::traceRelease("json:%s\n", folly::toJson(targetProfileInfo).c_str());
      }
    }
  }
}

struct SymbolFixup : boost::static_visitor<void> {
  SymbolFixup(ProfDataDeserializer& ser, StringData* name, uint32_t size) :
      ser{ser}, name{name}, size{size} {}

  template<typename T> void operator()(T&) { always_assert(false); }

  template<typename T>
  void go(rds::Profile& pt) {
    auto prof = TargetProfile<T>::deserialize(
      {pt.transId}, TransKind::Profile, pt.bcOff, name, size - sizeof(T));

    read_maybe_serializable(ser, prof.value());
    maybe_output_target_profile_trace(name, prof, pt);
  }

  void operator()(rds::Profile& pt) {
    switch (pt.kind) {
      case rds::ProfileKind::None: always_assert(false);
#define PR(T)                   \
      case rds::ProfileKind::T: \
        return go<T>(pt);
      RDS_PROFILE_SYMBOLS
#undef PR
    }
    not_reached();
  }

  ProfDataDeserializer& ser;
  StringData* name;
  // The size of the original rds allocation.
  uint32_t size;
};

void read_target_profiles(ProfDataDeserializer& ser) {
  BootStats::Block timer("DES_read_target_profiles",
                         RuntimeOption::ServerExecutionMode());
  while (true) {
    auto const size = read_raw<uint32_t>(ser);
    if (!size) break;
    auto const name = read_string(ser);

    // For now, we only write rds::Profile.
    rds::Profile profile{};
    profile.kind = read_raw<rds::ProfileKind>(ser);
    profile.transId = read_raw<TransID>(ser);
    profile.bcOff = read_raw<Offset>(ser);
    profile.name = read_string(ser);
    rds::Symbol sym{profile};
    auto sf = SymbolFixup{ser, name, size};
    boost::apply_visitor(sf, sym);
  }
}

void merge_loaded_units(int numWorkers) {
  BootStats::Block timer("DES_merge_loaded_units",
                         RuntimeOption::ServerExecutionMode());
  auto units = loadedUnitsRepoAuth();

  std::vector<VMWorker> workers;
  // Compute a batch size that causes each thread to process approximately 16
  // batches.  Even if the batches are somewhat imbalanced in what they contain,
  // the straggler workers are very unlikey to take more than 10% longer than
  // the first worker to finish.
  auto const batchSize{std::max(units.size() / numWorkers / 16, size_t(1))};
  std::atomic<size_t> index{0};
  std::atomic<uint32_t> curr_node{0};
  for (auto worker = 0; worker < numWorkers; ++worker) {
    WorkerSpec spec;
    spec.numaNode = next_numa_node(curr_node);
    workers.emplace_back(
      VMWorker(
        spec,
        [&] {
          ProfileNonVMThread nonVM;
#if USE_JEMALLOC_EXTENT_HOOKS
          if (auto arena = next_extra_arena(spec.numaNode)) {
            arena->bindCurrentThread();
          }
#endif
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
        }
      )
    );
  }
  for (auto& worker : workers) {
    worker.start();
  }
  for (auto& worker : workers) {
    worker.waitForEnd();
  }
}

////////////////////////////////////////////////////////////////////////////////

void renameFile(const std::string& src, const std::string& dst) {
  if (::rename(src.c_str(), dst.c_str()) != -1) return;

  auto const msg =
    folly::sformat("Failed to rename {} to {}, {}",
                   src, dst, folly::errnoStr(errno));
  Logger::Error(msg);
  throw std::runtime_error(msg);
};

std::string mangleFilenameForCreate(const std::string& name) {
  return name + ".part1";
}

std::string mangleFilenameForAppendStart(const std::string& name) {
  return name + ".part2";
}

std::string mangleFilenameForAppendInProgress(const std::string& name) {
  return name + ".part3";
}

////////////////////////////////////////////////////////////////////////////////
}

ProfDataSerializer::ProfDataSerializer(const std::string& name, FileMode mode)
  : baseFileName(name)
  , fileMode(mode) {

  auto const check = [] (int ret, const std::string& file) {
    if (ret != -1) return;
    auto const msg =
      folly::sformat("Failed to open file for write {}, {}", file,
                     folly::errnoStr(errno));
    Logger::Error(msg);
    throw std::runtime_error(msg);
  };

  if (fileMode == FileMode::Append) {
    fileName = mangleFilenameForAppendInProgress(name);
    auto const mangled = mangleFilenameForAppendStart(name);
    fd = open(mangled.c_str(),
              O_CLOEXEC | O_APPEND | O_WRONLY, 0644);
    check(fd, mangled);
    renameFile(mangled, fileName);
  } else {
    // Delete old profile data to avoid confusion.  This should've happened from
    // outside the process, but in case it didn't happen, try to do it here.
    unlink(name.c_str());

    fileName = mangleFilenameForCreate(name);
    fd = open(fileName.c_str(),
              O_CLOEXEC | O_CREAT | O_TRUNC | O_WRONLY, 0644);
    check(fd, fileName);
  }
}

void ProfDataSerializer::finalize() {
  assertx(fd != -1);
  if (offset) ::write(fd, buffer, offset);
  offset = 0;
  close(fd);
  fd = -1;

  // Rename file to its appropriate new name. If we're going to append
  // additional profile data, rename it to another (different)
  // temporary name (to mark the base data has been written). If not,
  // rename it to its file name (signifying completion).
  if (fileMode == FileMode::Create && serializeOptProfEnabled()) {
    // Don't rename the file to it's final name yet as we're still going to
    // append the profile data collected for the optimized code to it.
    FTRACE(1, "Finished serializing base profile data to {}\n", fileName);
    renameFile(fileName, mangleFilenameForAppendStart(baseFileName));
  } else {
    FTRACE(1, "Finished serializing all profile data to {}\n", fileName);
    renameFile(fileName, baseFileName);
  }
}

ProfDataSerializer::~ProfDataSerializer() {
  if (fd != -1) {
    // We didn't finalize(), maybe because an exception was thrown while writing
    // the data.  The file is likely corrupt or incomplete, so discard it.
    ftruncate(fd, 0);
    close(fd);
    unlink(fileName.c_str());
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
  auto const pos = lseek(fd, 0, SEEK_CUR);
  auto const end = lseek(fd, 0, SEEK_END);
  lseek(fd, pos, SEEK_SET); // go back to original position
  return offset == buffer_size && pos == end;
}

ProfDataSerializer::Mappers ProfDataDeserializer::getMappers() const {
  ProfDataSerializer::Mappers mappers;
  auto const rev = [] (auto const& i, auto& o) {
    for (ProfDataSerializer::Id id = 1; id < i.map.size(); ++id) {
      if (!i.map[id]) continue;
      o.assign(id, i.map[id]);
    }
  };
  rev(unitMap, mappers.unitMap);
  rev(funcMap, mappers.funcMap);
  rev(classMap, mappers.classMap);
  rev(typeAliasMap, mappers.typeAliasMap);
  rev(stringMap, mappers.stringMap);
  rev(arrayMap, mappers.arrayMap);
  rev(ratMap, mappers.ratMap);
  rev(funcIdMap, mappers.funcIdMap);
  return mappers;
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

Unit* ProfDataDeserializer::getUnit(ProfDataSerializer::Id id) const {
  return unitMap.get(id);
}

Func* ProfDataDeserializer::getFunc(ProfDataSerializer::Id id) const {
  return funcMap.get(id);
}

Class* ProfDataDeserializer::getClass(ProfDataSerializer::Id id) const {
  return classMap.get(id);
}

const TypeAlias*
ProfDataDeserializer::getTypeAlias(ProfDataSerializer::Id id) const {
  return typeAliasMap.get(id);
}

ArrayData* ProfDataDeserializer::getArray(ProfDataSerializer::Id id) const {
  return arrayMap.get(id);
}

StringData* ProfDataDeserializer::getString(ProfDataSerializer::Id id) const {
  return stringMap.get(id);
}

const RepoAuthType::Array*
ProfDataDeserializer::getArrayRAT(ProfDataSerializer::Id id) const {
  return ratMap.get(id);
}

FuncId ProfDataDeserializer::getFuncId(ProfDataSerializer::Id id) const {
  return FuncId::fromInt(funcIdMap.get(id));
}

void ProfDataDeserializer::record(ProfDataSerializer::Id id, Unit* unit) {
  unitMap.record(id, unit);
}

void ProfDataDeserializer::record(ProfDataSerializer::Id id, Func* func) {
  funcMap.record(id, func);
}

void ProfDataDeserializer::record(ProfDataSerializer::Id id, Class* cls) {
  classMap.record(id, cls);
}

void ProfDataDeserializer::record(ProfDataSerializer::Id id,
                                  const TypeAlias* ta) {
  typeAliasMap.record(id, ta);
}

void ProfDataDeserializer::record(ProfDataSerializer::Id id, ArrayData* arr) {
  arrayMap.record(id, arr);
}

void ProfDataDeserializer::record(ProfDataSerializer::Id id, StringData* str) {
  stringMap.record(id, str);
}

void ProfDataDeserializer::record(ProfDataSerializer::Id id,
                                  const RepoAuthType::Array* arr) {
  ratMap.record(id, arr);
}

void ProfDataDeserializer::record(ProfDataSerializer::Id id, FuncId funcId) {
  funcIdMap.record(id, funcId.toInt());
}

std::pair<ProfDataSerializer::Id, bool>
ProfDataSerializer::serialize(const Unit* unit) {
  return mappers.unitMap.get(unit);
}

std::pair<ProfDataSerializer::Id, bool>
ProfDataSerializer::serialize(const Func* func) {
  return mappers.funcMap.get(func);
}

std::pair<ProfDataSerializer::Id, bool>
ProfDataSerializer::serialize(const Class* cls) {
  return mappers.classMap.get(cls);
}

std::pair<ProfDataSerializer::Id, bool>
ProfDataSerializer::serialize(const TypeAlias* td) {
  return mappers.typeAliasMap.get(td);
}

std::pair<ProfDataSerializer::Id, bool>
ProfDataSerializer::serialize(const ArrayData* ad) {
  return mappers.arrayMap.get(ad);
}

std::pair<ProfDataSerializer::Id, bool>
ProfDataSerializer::serialize(const StringData* sd) {
  return mappers.stringMap.get(sd);
}

std::pair<ProfDataSerializer::Id, bool>
ProfDataSerializer::serialize(const RepoAuthType::Array* arr) {
  return mappers.ratMap.get(arr);
}

std::pair<ProfDataSerializer::Id, bool>
ProfDataSerializer::serialize(FuncId fid) {
  return mappers.funcIdMap.get(fid.toInt());
}

bool ProfDataSerializer::present(const Class* cls) const {
  return mappers.classMap.present(cls);
}

bool ProfDataSerializer::present(const TypeAlias* ta) const {
  return mappers.typeAliasMap.present(ta);
}

void write_raw_string(ProfDataSerializer& ser, const StringData* str) {
  uint32_t sz = str->size();
  write_raw(ser, sz);
  write_raw(ser, str->data(), sz);
  if (allowBespokeArrayLikes()) {
    write_raw(ser, str->color());
  }
}

StringData* read_raw_string(ProfDataDeserializer& ser,
                            bool skip /* = false */) {
  auto const sz = read_raw<uint32_t>(ser);
  constexpr uint32_t kMaxStringLen = StringData::MaxSize;
  if (sz > kMaxStringLen) {
    throw std::runtime_error("string too long, likely corrupt");
  }
  constexpr uint32_t kBufLen = 8192;
  char buffer[kBufLen];
  char* ptr = buffer;
  if (sz > kBufLen) ptr = (char*)malloc(sz);
  SCOPE_EXIT { if (ptr != buffer) free(ptr); };
  read_raw(ser, ptr, sz);
  auto const color = [&] {
    if (allowBespokeArrayLikes()) return read_raw<uint16_t>(ser);
    return uint16_t{0};
  }();
  if (!skip) {
    auto const str = makeStaticString(ptr, sz);
    str->setColor(color);
    return str;
  }
  return nullptr;
}

void write_string(ProfDataSerializer& ser, const StringData* str) {
  auto const [id, old] = ser.serialize(str);
  if (old) return write_id(ser, id);
  write_serialized_id(ser, id);
  write_raw_string(ser, str);
}

void write_string(ProfDataSerializer& ser, const std::string& str) {
  ITRACE(2, "cpp string>\n");
  uint64_t size = str.size();
  write_raw(ser, size);
  write_raw(ser, str.data(), size);
  ITRACE(2, "cpp string {}\n", str);
}

StringData* read_string(ProfDataDeserializer& ser) {
  return deserialize(
    ser,
    [&] { return read_raw_string(ser); }
  );
}

std::string read_cpp_string(ProfDataDeserializer& ser) {
  ITRACE(2, "cpp string>\n");
  auto const size = read_raw<uint64_t>(ser);
  constexpr uint32_t kMaxStringLen = 2 << 20;
  if (size > kMaxStringLen) {
    throw std::runtime_error("string too long, likely corrupt");
  }
  auto const buf = std::make_unique<char[]>(size);
  read_raw(ser, buf.get(), size);
  auto const res = std::string{buf.get(), size};
  ITRACE(2, "cpp string : {}\n", res);
  return res;
}

void write_array(ProfDataSerializer& ser, const ArrayData* arr) {
  auto const [id, old] = ser.serialize(arr);
  if (old) return write_id(ser, id);
  write_serialized_id(ser, id);
  auto const str = internal_serialize(VarNR(const_cast<ArrayData*>(arr)));
  uint32_t sz = str.size();
  write_raw(ser, sz);
  write_raw(ser, str.data(), sz);

  if (!allowBespokeArrayLikes()) return;
  write_layout(ser, ArrayLayout::FromArray(arr));
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
      auto const result = ArrayData::GetScalarArray(std::move(v));

      if (!allowBespokeArrayLikes()) return result;
      return read_layout(ser).apply(result);
    }
  );
}

void write_array_rat(ProfDataSerializer& ser, const RepoAuthType::Array* arr) {
  auto const [id, old] = ser.serialize(arr);
  if (old) return write_id(ser, id);
  write_serialized_id(ser, id);
  BlobEncoder encoder;
  arr->serialize(encoder);
  write_raw(ser, (uint32_t)encoder.size());
  write_raw(ser, encoder.data(), encoder.size());
}

const RepoAuthType::Array* read_array_rat(ProfDataDeserializer& ser) {
  return deserialize(
    ser,
    [&] () -> const RepoAuthType::Array* {
      auto const size = read_raw<uint32_t>(ser);
      constexpr uint32_t kBufLen = 8192;
      char buffer[kBufLen];
      char* ptr = buffer;
      if (size > kBufLen) ptr = (char*)malloc(size);
      SCOPE_EXIT { if (ptr != buffer) free(ptr); };
      read_raw(ser, ptr, size);
      BlobDecoder decoder{ptr, size};
      return RepoAuthType::Array::deserialize(decoder);
    }
  );
}

void write_unit(ProfDataSerializer& ser, const Unit* unit) {
  auto const [id, old] = ser.serialize(unit);
  if (old) return write_id(ser, id);
  ITRACE(2, "Unit: {}\n", unit->origFilepath());
  write_serialized_id(ser, id);
  write_string(ser, unit->origFilepath());
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
      return lookupUnit(filepath, "", nullptr, nullptr, false);
    }
  );
}

void write_class(ProfDataSerializer& ser, const Class* cls) {
  SCOPE_EXIT {
    ITRACE(2, "Class: {}\n", cls ? cls->name() : staticEmptyString());
  };
  ITRACE(2, "Class>\n");
  Trace::Indent _;

  auto const [id, old] = ser.serialize(cls);
  if (old) return write_id(ser, id);

  write_serialized_id(ser, id);
  write_string(ser, cls->preClass()->name());
  write_unit(ser, cls->preClass()->unit());

  jit::vector<const Class*> dependents;
  auto record_dep = [&] (const Class* dep) {
    if (!dep) return;
    if (!ser.present(dep) || !classHasPersistentRDS(dep)) {
      dependents.emplace_back(dep);
    }
  };
  record_dep(cls->parent());
  for (auto const& iface : cls->declInterfaces()) {
    record_dep(iface.get());
  }

  if (cls->preClass()->attrs() & AttrNoExpandTrait) {
    for (auto const tName : cls->preClass()->usedTraits()) {
      auto const trait =
        Class::lookupUniqueInContext(tName, nullptr, nullptr);
      assertx(trait);
      record_dep(trait);
    }
  } else {
    for (auto const& trait : cls->usedTraitClasses()) {
      record_dep(trait.get());
    }
  }

  if (cls->hasIncludedEnums()) {
    for (auto const& e : cls->allIncludedEnums().range()) {
      record_dep(e.get());
    }
  }

  write_container(ser, dependents, write_class);

  if (cls->attrs() & AttrEnum &&
      cls->preClass()->enumBaseTy().isUnresolved()) {
    write_named_type(ser, cls->preClass()->enumBaseTy().typeNamedType());
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

void write_lclass(ProfDataSerializer& ser, LazyClassData lcls) {
  ITRACE(2, "LClass>\n");
  Trace::Indent _i;
  write_raw_string(ser, lcls.name());
  ITRACE(2, "LCLass: {}\n", lcls.name());
}

LazyClassData read_lclass(ProfDataDeserializer& ser) {
  ITRACE(2, "LClass>\n");
  auto const name = read_raw_string(ser);
  ITRACE(2, "LClass: {}\n", name ? name : staticEmptyString());
  return LazyClassData::create(name);
}

void write_typealias(ProfDataSerializer& ser, const TypeAlias* td) {
  SCOPE_EXIT {
    ITRACE(2, "TypeAlias: {}\n", td->name());
  };
  ITRACE(2, "TypeAlias>\n");
  Trace::Indent _;

  auto const [id, old] = ser.serialize(td);
  if (old) return write_id(ser, id);

  write_serialized_id(ser, id);

  auto const tdId = [td] {
    Id tdId = 0;
    auto const name = td->name();
    for (auto const& ta : td->unit()->typeAliases()) {
      if (ta.name == name) return tdId;
      tdId++;
    }
    always_assert(false);
  }();

  write_raw(ser, tdId);
  write_unit(ser, td->unit());
  write_raw(ser, td->value);
}

const TypeAlias* read_typealias(ProfDataDeserializer& ser) {
  ITRACE(2, "TypeAlias>\n");
  auto const ret = deserialize(
    ser,
    [&] () -> const TypeAlias* {
      Trace::Indent _;
      return read_typealias_internal(ser);
    }
  );

  ITRACE(2, "TypeAlias: {}\n", ret ? ret->name() : staticEmptyString());
  return ret;
}

void write_func(ProfDataSerializer& ser, const Func* func) {
  SCOPE_EXIT {
    ITRACE(2, "Func: {}\n", func ? func->fullName() : staticEmptyString());
  };
  ITRACE(2, "Func>\n");
  Trace::Indent _;

  auto const [id, old] = ser.serialize(func);
  if (old) return write_id(ser, id);

  write_serialized_id(ser, id);
  write_func_id(ser, func->getFuncId());

  if (func == SystemLib::s_nullCtor ||
      (!func->isMethod() && func->isBuiltin() &&
      !func->isMethCaller())) {
    if (func == SystemLib::s_nullCtor) {
      assertx(func->name() == s_86ctor.get());
    }
    write_raw(ser, true);
    return write_string(ser, func->name());
  }
  write_raw(ser, false);

  if (func->isMethod()) {
    auto const* cls = func->implCls();
    auto const nslot = [&] () -> uint32_t {
      const uint32_t slot = func->methodSlot();
      if (cls->getMethodSafe(slot) != func) {
        if (func->name() == s_86pinit.get()) return k86pinitSlot;
        if (func->name() == s_86sinit.get()) return k86sinitSlot;
        if (func->name() == s_86linit.get()) return k86linitSlot;
        cls = func->cls();
        assertx(cls->getMethod(slot) == func);
      }
      return ~slot;
    }();
    assertx(nslot & 0x80000000);
    write_raw(ser, nslot);
    write_class(ser, cls);
    return;
  }

  const uint32_t sn = func->sn();
  assertx(!(sn & 0x80000000));
  write_raw(ser, sn);
  write_unit(ser, func->unit());
}

Func* read_func(ProfDataDeserializer& ser) {
  ITRACE(2, "Func>\n");
  auto const ret = deserialize(
    ser,
    [&] () -> Func* {
      Trace::Indent _;
      auto const fid = read_id(ser);
      auto const builtin_func = read_raw<bool>(ser);
      auto const func = [&] () -> const Func* {
        if (builtin_func) {
          auto const name = read_string(ser);
          if (name->same(s_86ctor.get())) return SystemLib::s_nullCtor;
          return Func::lookup(name);
        }
        auto const id = read_raw<uint32_t>(ser);
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
          if (f->sn() == id) {
            auto const ne = f->getNamedFunc();
            // If it's not persistent, make sure its NamedFunc is
            // unbound, ready for DefFunc
            if (ne->m_cachedFunc.bound() && ne->m_cachedFunc.isNormal()) {
              ne->m_cachedFunc.markUninit();
            }
            Func::def(f);
            return f;
          }
        }
        not_reached();
      }();
      ser.record(fid, func->getFuncId());
      return const_cast<Func*>(func);
    }
  );
  ITRACE(2, "Func: {}\n", ret ? ret->fullName() : staticEmptyString());
  return ret;
}

void write_clsmeth(ProfDataSerializer& ser, ClsMethDataRef clsMeth) {
  SCOPE_EXIT {
    ITRACE(2, "ClsMeth: {}, {}\n",
      clsMeth->getCls() ? clsMeth->getCls()->name() : staticEmptyString(),
      clsMeth->getFunc() ? clsMeth->getFunc()->fullName() : staticEmptyString()
    );
  };
  ITRACE(2, "ClsMeth>\n");
  write_class(ser, clsMeth->getCls());
  write_func(ser, clsMeth->getFunc());
}

ClsMethDataRef read_clsmeth(ProfDataDeserializer& ser) {
  ITRACE(2, "ClsMeth>\n");
  auto const cls = read_class(ser);
  auto const func = read_func(ser);
  ITRACE(2, "ClsMeth: {}, {}\n",
    cls ? cls->name() : staticEmptyString(),
    func ? func->fullName() : staticEmptyString());
  return ClsMethDataRef::create(cls, func);
}

void write_regionkey(ProfDataSerializer& ser, const RegionEntryKey& regionkey) {
  write_srckey(ser, regionkey.srcKey());
  write_container(ser, regionkey.guards(), write_guarded_location);
}

RegionEntryKey read_regionkey(ProfDataDeserializer& des) {
  auto srcKey = read_srckey(des);
  GuardedLocations guards;
  read_container(des,
                 [&] {
                   guards.push_back(read_guarded_location(des));
                 });
  return RegionEntryKey(srcKey, guards);
}

void write_prologueid(ProfDataSerializer& ser, const PrologueID& pid) {
  ITRACE(2, "PrologueID>\n");
  write_func(ser, pid.func());
  write_raw(ser, safe_cast<uint32_t>(pid.nargs()));
  ITRACE(2, "PrologueID: {}\n", show(pid));
}

PrologueID read_prologueid(ProfDataDeserializer& des) {
  ITRACE(2, "PrologueID>\n");
  auto const func = read_func(des);
  auto const nargs = read_raw<uint32_t>(des);
  auto const pid = PrologueID(func, nargs);
  ITRACE(2, "PrologueID: {}\n", show(pid));
  return pid;
}

void write_srckey(ProfDataSerializer& ser, SrcKey sk) {
  ITRACE(2, "SrcKey>\n");
  write_func(ser, sk.func());
  write_raw(ser, sk.toAtomicInt());
  ITRACE(2, "SrcKey: {}\n", show(sk));
}

SrcKey read_srckey(ProfDataDeserializer& ser) {
  ITRACE(2, "SrcKey>\n");
  auto const func = read_func(ser);
  auto const orig = SrcKey::fromAtomicInt(read_raw<SrcKey::AtomicInt>(ser));
  auto const sk = orig.withFuncID(func->getFuncId());
  ITRACE(2, "SrcKey: {}\n", show(sk));
  return sk;
}

void write_layout(ProfDataSerializer& ser, ArrayLayout layout) {
  write_raw(ser, layout.toUint16());
}

ArrayLayout read_layout(ProfDataDeserializer& ser) {
  return ArrayLayout::FromUint16(read_raw<uint16_t>(ser));
}

void write_func_id(ProfDataSerializer& ser, FuncId fid) {
  write_id(ser, ser.serialize(fid).first);
}

FuncId read_func_id(ProfDataDeserializer& ser) {
  auto const id = read_id(ser);
  return ser.getFuncId(id);
}

namespace {

ProfDataSerializer::Mappers s_lastMappers;

}

std::string serializeProfData(const std::string& filename) {
  try {
    ProfDataSerializer ser{filename, ProfDataSerializer::FileMode::Create};

    write_raw(ser, kMagic);
    write_raw(ser, RepoFile::globalData().Signature);
    auto schema = repoSchemaId();
    write_raw(ser, schema.size());
    write_raw(ser, schema.begin(), schema.size());

    auto host = Process::GetHostName();
    write_raw(ser, host.size());
    write_raw(ser, &host[0], host.size());
    auto& tag = RuntimeOption::ProfDataTag;
    write_raw(ser, tag.size());
    write_raw(ser, &tag[0], tag.size());
    write_raw(ser, TimeStamp::Current());

    Func::s_treadmill = true;
    hphp_session_init(Treadmill::SessionKind::ProfData);
    requestInitProfData();

    SCOPE_EXIT {
      requestExitProfData();
      hphp_context_exit();
      hphp_session_exit();
      Func::s_treadmill = false;
    };

    write_rds_ordering(ser);

    write_units_preload(ser);
    ExtensionRegistry::serialize(ser);
    PropertyProfile::serialize(ser);
    InstanceBits::init();
    InstanceBits::serialize(ser);

    auto const pd = profData();
    write_prof_data(ser, pd);

    if (RO::EnableIntrinsicsExtension) {
      write_container(ser, prioritySerializeClasses(), write_class);
    }
    write_container(ser, Class::serializeLazyAPCClasses(), write_class);

    write_target_profiles(ser);

    // We've written everything directly referenced by the profile
    // data, but jitted code might still use Classes and TypeAliases
    // that haven't been otherwise mentioned (eg TypeConstraint types,
    // InstanceOfD etc).
    write_seen_types(ser, pd);

    if (allowBespokeArrayLikes()) {
      serializeBespokeLayouts(ser);
    }

    writeGlobalProfiles(ser);

    // Record the size of main profile code so we can use it to fake
    // jit.maturity if we're going to restart before running RTA.
    write_raw(ser, tc::getProfMainUsage());

    ser.finalize();

    if (serializeOptProfEnabled()) {
      s_lastMappers = std::move(ser.getMappers());
    }

    return "";
  } catch (std::runtime_error& err) {
    return folly::sformat("Failed to serialize profile data: {}", err.what());
  }
}

std::string serializeOptProfData(const std::string& filename) {
  try {
    ProfDataSerializer ser(filename, ProfDataSerializer::FileMode::Append);

    ser.setMappers(std::move(s_lastMappers));

    // If enabled, recompute the function order using the call graph obtained
    // via instrumentation of the optimized code, then serialize it.
    if (Cfg::Jit::PGOOptCodeCallGraph) {
      FuncOrder::compute();
    }
    FuncOrder::serialize(ser);

    // Serialize the vasm block counters.
    VasmBlockCounters::serialize(ser);

    serializeCachedInliningCost(ser);

    ser.finalize();

    return "";
  } catch (std::runtime_error& err) {
    return folly::sformat("Failed serializeOptProfData: {}", err.what());
  }
}

std::string deserializeProfData(const std::string& filename,
                                int numWorkers,
                                bool rds) {
  s_deserializedFile = filename;
  try {
    if (!rds) ProfData::setTriedDeserialization();

    ProfDataDeserializer ser{filename};

    if (read_raw<decltype(kMagic)>(ser) != kMagic) {
      throw std::runtime_error("Not a profile-data dump");
    }
    auto signature = read_raw<decltype(RepoFile::globalData().Signature)>(ser);
    if (signature != RepoFile::globalData().Signature) {
      auto const msg =
        folly::sformat("Mismatched repo signature (expected signature '{}', got '{}')",
                       RepoFile::globalData().Signature, signature);

      throw std::runtime_error(msg);
    }
    auto size = read_raw<size_t>(ser);
    std::string schema;
    schema.resize(size);
    read_raw(ser, &schema[0], size);
    if (schema != repoSchemaId()) {
      auto const msg =
        folly::sformat("Mismatched repo-schema (expected schema_id '{}', got '{}')",
          repoSchemaId(), schema);

      throw std::runtime_error(msg);
    }

    size = read_raw<size_t>(ser);
    std::string buildHost;
    buildHost.resize(size);
    read_raw(ser, &buildHost[0], size);

    size = read_raw<size_t>(ser);
    std::string tag;
    tag.resize(size);
    read_raw(ser, &tag[0], size);

    int64_t buildTime;
    read_raw(ser, buildTime);
    auto const currTime = TimeStamp::Current();
    if (buildTime <= currTime - 3600 * RuntimeOption::ProfDataTTLHours) {
      throw std::runtime_error(
          "Stale profile data (check Eval.ProfDataTTLHours)");
    } else if (buildTime > currTime) {
      throw std::runtime_error(
          folly::sformat("profile data build timestame: {}, currTime: {}",
                         buildTime, currTime).c_str());
    }

    // If we're loading RDS ordering, we can stop here.
    auto const ordering = read_rds_ordering(ser);
    if (rds) {
      rds::setPreAssignments(ordering);
      return "";
    }

    read_units_preload(ser);
    ExtensionRegistry::deserialize(ser);
    PropertyProfile::deserialize(ser);
    InstanceBits::deserialize(ser);

    ProfData::Session pds;
    auto const pd = profData();
    read_prof_data(ser, pd);
    pd->setDeserialized(buildHost, tag, buildTime);

    // Optionally log function info from jit profile
    if (RuntimeOption::EvalDumpJitProfileStats) {
      auto const logFunc = [&](auto const& func) {
        StructuredLogEntry entry;
        entry.force_init = true;
        entry.setInt("signature", signature);
        entry.setStr("repo_schema", schema);
        entry.setStr("build_host", buildHost);
        entry.setInt("build_time", buildTime);
        entry.setStr("orig_filepath", func->unit()->origFilepath()->data());
        entry.setStr("function_name", func->fullName()->data());
        entry.setInt("function_bytecode_hash", func->bcHash());
        StructuredLog::log("hhvm_jit_profile_stats", entry);
      };

      pd->forEachProfilingFunc(logFunc);
    }

    if (RO::EnableIntrinsicsExtension) {
      read_container(ser, [&] { read_class(ser); });
    }
    {
      std::vector<const Class*> list;
      read_container(ser, [&] { list.push_back(read_class(ser)); });
      Class::deserializeLazyAPCClasses(list);
    }

    read_target_profiles(ser);

    read_seen_types(ser);

    if (allowBespokeArrayLikes()) {
      deserializeBespokeLayouts(ser);
    }

    readGlobalProfiles(ser);

    // If isJitSerializing() is true, we've restarted to reload the
    // profiling data. Record the size of prof before we restarted so
    // we can fake jit.maturity.
    auto const prevProfSize = read_raw<size_t>(ser);
    if (isJitSerializing()) ProfData::setPrevProfSize(prevProfSize);

    if (!ser.done()) {
      // We have profile data for the optimized code, so deserialize it too.
      FuncOrder::deserialize(ser);
      VasmBlockCounters::deserialize(ser);
      deserializeCachedInliningCost(ser);
    }

    if (s_preload_dispatcher) {
      BootStats::Block timer("DES_wait_for_units_preload",
                             RuntimeOption::ServerExecutionMode());
      s_preload_dispatcher->waitEmpty(true);
      delete s_preload_dispatcher;
      s_preload_dispatcher = nullptr;
    }
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

    if (isJitSerializing() && serializeOptProfEnabled()) {
      s_lastMappers = ser.getMappers();
    }

    return "";
  } catch (std::runtime_error& err) {
    return folly::sformat("Failed to deserialize profile data {}: {}",
                          filename, err.what());
  }
}

bool tryDeserializePartialProfData(const std::string& filename,
                                   int numWorkers,
                                   bool rds) {
  auto const mangled = mangleFilenameForAppendStart(filename);
  return deserializeProfData(mangled, numWorkers, rds).empty();
}

bool serializeOptProfEnabled() {
  return Cfg::Jit::SerializeOptProfSeconds  > 0 ||
         Cfg::Jit::SerializeOptProfRequests > 0;
}

Optional<std::string> getFilenameDeserialized() {
  return s_deserializedFile;
}

//////////////////////////////////////////////////////////////////////
} }
