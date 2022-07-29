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
#include "hphp/runtime/vm/hackc-translator.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/opcodes.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/runtime/vm/unit-gen-helpers.h"
#include "hphp/zend/zend-string.h"

#include <folly/Range.h>

namespace HPHP {

TRACE_SET_MOD(hackc_translate);

namespace {

using namespace HPHP::hackc;
using namespace HPHP::hackc::hhbc;

struct TranslationException : Exception {
  template<class... A>
  explicit TranslationException(A&&... args)
    : Exception(folly::sformat(std::forward<A>(args)...))
  {}
};

[[noreturn]] void error(const char* what) {
  throw TranslationException("{}: {}", what, folly::errnoStr(errno));
}

struct TranslationState {
  void enterCatch() {
    handler.push(fe->bcPos());
  }

  void enterTry() {
    start.push(fe->bcPos());
  }

  void finishTryCatch() {
    assertx(!start.empty());
    assertx(!handler.empty());
    auto& eh = fe->addEHEnt();
    eh.m_base = start.top();
    eh.m_past = handler.top();
    eh.m_iterId = -1;
    eh.m_handler = handler.top();
    eh.m_end = fe->bcPos();
    start.pop();
    handler.pop();
  }

  Id mergeLitstr(const StringData* sd) {
    auto const id = ue->mergeLitstr(sd);
    litstrMap.emplace(id, sd);
    return id;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Opcode helpers

  void addLabelJump(const Dummy&);
  void addLabelJump(const Label& label);
  void addLabelJump(const Label label[2]);

  HPHP::RepoAuthType translateRepoAuthType(folly::StringPiece str);
  HPHP::MemberKey translateMemberKey(const hhbc::MemberKey& mkey);
  ArrayData* getArrayfromAdataId(const AdataId& id);

  ///////////////////////////////////////////////////////////////////////////////

  // Locals are 0-indexed u32. In order to differentiate seeing no
  // locals vs seeing a local with id 0, maxUnnamed is max local
  // id + 1.
  void trackMaxUnnamed(uint32_t loc) {
    if (loc + 1 > maxUnnamed) maxUnnamed = loc + 1;
  }

  UnitEmitter* ue;
  FuncEmitter* fe{nullptr};
  PreClassEmitter* pce{nullptr};
  // Map of adata identifiers to their associated static arrays.
  std::map<std::string, ArrayData*> adataMap;

  // Used for Execption Handler entry.
  jit::stack<Offset> handler;
  jit::stack<Offset> start;

  struct LabelInfo {
    // Target is the offset of the Label opcode
    Offset target;
    std::vector<Label> dvInits;
    // First offset is the immediate offset of a Label, like a jump target.
    // Second offset is the opcode containing the label immediate.(ie
    // a jmp instruction itself). This is used for computing jmp delta to
    // patch Label immediate.
    std::vector<std::pair<Offset,Offset>> sources;
  };

  std::map<Label, LabelInfo> labelMap;

  // In whole program mode it isn't possible to lookup a litstr in the global
  // table while emitting, so keep a lookaside of litstrs seen by the assembler.
  std::unordered_map<Id, const StringData*> litstrMap;

  // Max local id encountered. Must be unsigned to match u32 representation
  // in HackC.
  uint32_t maxUnnamed{0};
  Location::Range srcLoc{-1,-1,-1,-1};
  std::vector<std::pair<hhbc::Label, Offset>> labelJumps;
};

///////////////////////////////////////////////////////////////////////////////
// hhbc::Slice helpers

template <class T>
folly::Range<const T*> range(Slice<T> const& s) {
  return folly::range(s.data, s.data + s.len);
}

template <class T>
folly::Range<const T*> range(BumpSliceMut<T> const& s) {
  return folly::range(s.data, s.data + s.len);
}

///////////////////////////////////////////////////////////////////////////////
// hhbc::Maybe helpers

template<typename T>
Optional<T> maybe(hackc::Maybe<T> m) {
  if (m.tag == hackc::Maybe<T>::Tag::Nothing) return std::nullopt;
  return m.Just._0;
}

template<typename T, typename Fn, typename ElseFn>
auto maybeOrElse(hackc::Maybe<T> m, Fn fn, ElseFn efn) {
  auto opt = maybe(m);
  return opt ? fn(opt.value()) : efn();
}

template<typename T, typename Fn>
auto maybeThen(hackc::Maybe<T> m, Fn fn) {
  auto opt = maybe(m);
  if (opt) fn(opt.value());
}

///////////////////////////////////////////////////////////////////////////////

namespace {
struct FatalUnitError : std::runtime_error {
  FatalUnitError(
    const std::string& msg,
    const StringData* filePath,
    Location::Range pos,
    FatalOp op)
    : std::runtime_error(msg)
    , filePath(filePath)
    , pos(pos)
    , op(op)
  {}

  const StringData* filePath;
  Location::Range pos;
  FatalOp op;
};
}

///////////////////////////////////////////////////////////////////////////////
// hhbc::Str Helpers

std::string toString(const Str& str) {
  assertx(str.data != nullptr);
  return std::string{(const char*)str.data, str.len};
}

folly::StringPiece toStringPiece(const Str& str) {
  assertx(str.data != nullptr);
  return folly::StringPiece{(const char*)str.data, str.len};
}

StringData* toStaticString(const Str& str, uint32_t start = 0) {
  assertx(start <= str.len);
  return makeStaticString((char*)str.data + start, str.len - start);
}

// TODO(@voork): NamedLocals are still prefixed with '$'.
StringData* toNamedLocalStaticString(const Str& str) {
  return toStaticString(str, 1);
}

StringData* makeDocComment(const Str& str) {
  if (RuntimeOption::EvalGenerateDocComments) return toStaticString(str);
  return staticEmptyString();
}

///////////////////////////////////////////////////////////////////////////////

using kind = hhbc::TypedValue::Tag;

HPHP::TypedValue toTypedValue(const hackc::hhbc::TypedValue& tv) {
  auto hphp_tv = [&]() {
    switch(tv.tag) {
      case kind::Uninit:
        return make_tv<KindOfUninit>();
      case kind::Int:
        return make_tv<KindOfInt64>(tv.Int._0);
      case kind::Bool:
        return make_tv<KindOfBoolean>(tv.Bool._0);
      case kind::Float: {
        return make_tv<KindOfDouble>(tv.Float._0);
      }
      case kind::String: {
        auto const s = toStaticString(tv.String._0);
        return make_tv<KindOfPersistentString>(s);
      }
      case kind::Null:
        return make_tv<KindOfNull>();
      case kind::Vec: {
        VecInit v(tv.Vec._0.len);
        auto set = range(tv.Vec._0);
        for (auto const& elt : set) {
          v.append(toTypedValue(elt));
        }
        auto tv = v.create();
        ArrayData::GetScalarArray(&tv);
        return make_tv<KindOfVec>(tv);
      }
      case kind::Dict: {
        DictInit d(tv.Dict._0.len);
        auto set = range(tv.Dict._0);
        for (auto const& elt : set) {
          switch (elt._0.tag) {
            case kind::Int:
              d.set(elt._0.Int._0, toTypedValue(elt._1));
              break;
            case kind::String: {
              auto const s = toStaticString(elt._0.String._0);
              d.set(s, toTypedValue(elt._1));
              break;
            }
            case kind::LazyClass:{
              if (RuntimeOption::EvalRaiseClassConversionWarning) {
                raise_class_to_string_conversion_warning();
              }
              auto const s = toStaticString(elt._0.LazyClass._0);
              d.set(s, toTypedValue(elt._1));
              break;
            }
            default:
              always_assert(false);
          }
        }
        auto tv = d.create();
        ArrayData::GetScalarArray(&tv);
        return make_tv<KindOfDict>(tv);
      }
      case kind::Keyset: {
        KeysetInit k(tv.Keyset._0.len);
        auto set = range(tv.Keyset._0);
        for (auto const& elt : set) {
          k.add(toTypedValue(elt));
        }
        auto tv = k.create();
        ArrayData::GetScalarArray(&tv);
        return make_tv<KindOfKeyset>(tv);
      }
      case kind::LazyClass: {
        auto const lc = LazyClassData::create(toStaticString(tv.LazyClass._0));
        return make_tv<KindOfLazyClass>(lc);
      }
    }
    not_reached();
  }();
  checkSize(hphp_tv, RuntimeOption::EvalAssemblerMaxScalarSize);
  return hphp_tv;
}

///////////////////////////////////////////////////////////////////////////////
// Field translaters

void translateUserAttributes(Slice<HhasAttribute> attributes, UserAttributeMap& userAttrs) {
  Trace::Indent indent;
  auto attrs = range(attributes);
  for (auto const& attr : attrs) {
    auto const name = toStaticString(attr.name);
    VecInit v(attr.arguments.len);
    auto args = range(attr.arguments);
    for (auto const& arg : args) {
      v.append(toTypedValue(arg));
    }
    auto tv = v.create();
    ArrayData::GetScalarArray(&tv);
    userAttrs[name] = make_tv<KindOfVec>(tv);
  };
}

template<bool isEnum=false>
std::pair<const StringData*, TypeConstraint> translateTypeInfo(const HhasTypeInfo& t) {
  auto const user_type = maybeOrElse(t.user_type,
    [&](Str& s) {return toStaticString(s);},
    [&]() {return staticEmptyString();});

  auto const type_name = isEnum
    ? user_type
    : maybeOrElse(t.type_constraint.name,
        [&](Str& s) {return toStaticString(s);},
        [&]() {return nullptr;});

  auto flags = t.type_constraint.flags;
  return std::make_pair(user_type, TypeConstraint{type_name, flags});
}

void translateTypedef(TranslationState& ts, const HhasTypedef& t) {
  UserAttributeMap userAttrs;
  translateUserAttributes(t.attributes, userAttrs);
  auto attrs = t.attrs;
  if (!SystemLib::s_inited) attrs |= AttrPersistent;
  auto const name = toStaticString(t.name._0);

  auto const ty = translateTypeInfo(t.type_info).second;
  auto tname = ty.typeName();
  if (!tname) tname = staticEmptyString();

  auto tys = toTypedValue(t.type_structure);
  assertx(isArrayLikeType(tys.m_type));
  tvAsVariant(&tys).setEvalScalar();

  auto te = ts.ue->newTypeAliasEmitter(name->toCppString());
  te->init(
    t.span.line_begin,
    t.span.line_end,
    attrs,
    tname,
    tname->empty() ? AnnotType::Mixed : ty.type(),
    (ty.flags() & TypeConstraintFlags::Nullable) != 0,
    ArrNR{tys.m_data.parr},
    Array{}
  );

  te->setUserAttributes(userAttrs);
}

template<bool isType>
void addConstant(TranslationState& ts,
                 const StringData* name,
                 Optional<hhbc::TypedValue> tv,
                 bool isAbstract) {
  auto const kind = isType
    ? ConstModifiers::Kind::Type
    : ConstModifiers::Kind::Value;

  if (!tv) {
    assertx(isAbstract);
    ts.pce->addAbstractConstant(name, kind, false);
    return;
  }

  auto tvInit = toTypedValue(tv.value());
  tvAsVariant(&tvInit).setEvalScalar();
  ts.pce->addConstant(name,
                      nullptr,
                      &tvInit,
                      Array{},
                      kind,
                      PreClassEmitter::Const::Invariance::None,
                      false,
                      isAbstract);
}

void translateClassConstant(TranslationState& ts, const HhasConstant& c) {
  auto const name = toStaticString(c.name._0);
  auto const tv = maybe(c.value);
  addConstant<false>(ts, name, tv, c.is_abstract);
}

void translateTypeConstant(TranslationState& ts, const HhasTypeConstant& c) {
  auto const name = toStaticString(c.name);
  auto const tv = maybe(c.initializer);
  addConstant<true>(ts, name, tv, c.is_abstract);
}

void translateCtxConstant(TranslationState& ts, const HhasCtxConstant& c) {
  auto const name = toStaticString(c.name);
  bool isAbstract = c.is_abstract;
  auto coeffects = PreClassEmitter::Const::CoeffectsVec{};

  auto recognized = range(c.recognized);
  for (auto const& r : recognized) {
    coeffects.push_back(toStaticString(r));
  }
  auto unrecognized = range(c.unrecognized);
  for (auto const& u : unrecognized) {
    coeffects.push_back(toStaticString(u));
  }

  // T112974443: temporarily drop the abstract ones until runtime is fixed
  if (isAbstract && !RuntimeOption::EvalEnableAbstractContextConstants) return;
  DEBUG_ONLY auto added =
    ts.pce->addContextConstant(name, std::move(coeffects), isAbstract);
  assertx(added);
}

void translateProperty(TranslationState& ts, const HhasProperty& p, const UpperBoundMap& classUbs) {
  UserAttributeMap userAttributes;
  translateUserAttributes(p.attributes, userAttributes);

  auto const heredoc = maybeOrElse(p.doc_comment,
    [&](Str& s) {return makeDocComment(s);},
    [&]() {return staticEmptyString();});

  auto [userTy, typeConstraint] = translateTypeInfo(p.type_info);

  auto const hasReifiedGenerics =
    userAttributes.find(s___Reified.get()) != userAttributes.end();

  // T112889109: Passing in {} as the third argument here exposes a gcc compiler bug.
  auto ub = getRelevantUpperBounds(typeConstraint, classUbs, classUbs, {});

  auto needsMultiUBs = false;
  if (ub.size() == 1 && !hasReifiedGenerics) {
    applyFlagsToUB(ub[0], typeConstraint);
    typeConstraint = ub[0];
  } else if (!ub.empty()) {
    needsMultiUBs = true;
  }

  auto const tv = maybeOrElse(p.initial_value,
    [&](hhbc::TypedValue& s) {
      auto hphpTv = toTypedValue(s);
      tvAsVariant(&hphpTv).setEvalScalar();
      return hphpTv;
    },
    [&]() {return make_tv<KindOfNull>();});

  auto const name = toStaticString(p.name._0);
  ITRACE(2, "Translating property {} {}\n", name, tv.pretty());

  ts.pce->addProperty(name,
                      p.flags,
                      userTy,
                      typeConstraint,
                      needsMultiUBs ? std::move(ub) : UpperBoundVec{},
                      heredoc,
                      &tv,
                      HPHP::RepoAuthType{},
                      userAttributes);
}

void translateClassBody(TranslationState& ts,
                        const HhasClass& c,
                        const UpperBoundMap& classUbs) {
  auto props = range(c.properties);
  for (auto const& p : props) {
    translateProperty(ts, p, classUbs);
  }
  auto constants = range(c.constants);
  for (auto const& cns : constants) {
    translateClassConstant(ts, cns);
  }
  auto tconstants = range(c.type_constants);
  for (auto const& t : tconstants) {
    translateTypeConstant(ts, t);
  }
  auto cconstants = range(c.ctx_constants);
  for (auto const& ctx : cconstants) {
    translateCtxConstant(ts, ctx);
  }
}

using TypeInfoPair = Pair<Str, Slice<HhasTypeInfo>>;

void translateUbs(const TypeInfoPair& ub, UpperBoundMap& ubs) {
  auto const& name = toStaticString(ub._0);
  CompactVector<TypeConstraint> ret;

  auto infos = range(ub._1);
  for (auto const& i : infos) {
    ubs[name].emplace_back(translateTypeInfo(i).second);
  }
}

void translateEnumType(TranslationState& ts, const Maybe<HhasTypeInfo>& t) {
  auto const tOpt = maybe(t);
  if (tOpt) {
    ts.pce->setEnumBaseTy(translateTypeInfo<true>(tOpt.value()).second);
  }
}

void translateRequirements(TranslationState& ts, Pair<ClassName, TraitReqKind> requirement) {
  auto const name = toStaticString(requirement._0._0);
  auto const requirementKind = [&] {
    switch (requirement._1) {
      case TraitReqKind::MustExtend: return PreClass::RequirementExtends;
      case TraitReqKind::MustImplement: return PreClass::RequirementImplements;
      case TraitReqKind::MustBeClass: return PreClass::RequirementClass;
    }
    not_reached();
  }();

  ts.pce->addClassRequirement(PreClass::ClassRequirement(name, requirementKind));
}

HPHP::RepoAuthType TranslationState::translateRepoAuthType(folly::StringPiece str) {
  using T = HPHP::RepoAuthType::Tag;

  auto const [tag, what] = [&] {
    #define TAG(x, name) \
      if (str.startsWith(name)) return std::make_pair(T::x, name);
      REPO_AUTH_TYPE_TAGS(TAG)
    #undef TAG
    not_reached();
  }();

  if (HPHP::RepoAuthType::tagHasName(tag)) {
    str.removePrefix(what);
    auto const name = makeStaticString(str.data());
    mergeLitstr(name);
    return HPHP::RepoAuthType{tag, name};
  }

  return HPHP::RepoAuthType{tag};
}

////////////////////////////////////////////////////////////////////

// Do nothing for dummy values. Used for MemoGetEager where both labels
// are passed through first BA.
void TranslationState::addLabelJump(const Dummy&) {}

void TranslationState::addLabelJump(const Label& label) {
  labelJumps.emplace_back(label, fe->bcPos());
  fe->emitInt32(0);
}

// MemoGetEager passes both labels in first BA opcode
void TranslationState::addLabelJump(const Label label[2]) {
  addLabelJump(label[0]);
  addLabelJump(label[1]);
}

// labelOpcodeOff is the offset of the label opcode.
// immOff is the offset where the label appears as an immediate, ie as a Jump target.
void updateLabelSourceforJump(TranslationState& ts,
                              const Label& label,
                              Offset immOff,
                              Offset labelOpcodeOff) {
  auto& labelInfo = ts.labelMap[label];
  labelInfo.sources.emplace_back(immOff, labelOpcodeOff);
}

HPHP::MemberKey TranslationState::translateMemberKey(const hhbc::MemberKey& mkey) {
  switch (mkey.tag) {
    case hhbc::MemberKey::Tag::EC: {
      auto const iva = static_cast<int32_t>(mkey.EC._0);
      return HPHP::MemberKey(MemberCode::MEC, iva, mkey.EC._1);
    }
    case hhbc::MemberKey::Tag::PC: {
      auto const iva = static_cast<int32_t>(mkey.PC._0);
      return HPHP::MemberKey(MemberCode::MPC, iva, mkey.PC._1);
    }
    case hhbc::MemberKey::Tag::EL: {
      auto const id = static_cast<int32_t>(mkey.EL._0.idx);
      NamedLocal local{ id, id };
      trackMaxUnnamed(id);
      return HPHP::MemberKey(MemberCode::MEL, local, mkey.EL._1);
    }
    case hhbc::MemberKey::Tag::PL: {
      auto const id = static_cast<int32_t>(mkey.PL._0.idx);
      NamedLocal local{ id, id };
       trackMaxUnnamed(id);
      return HPHP::MemberKey(MemberCode::MPL, local, mkey.PL._1);
    }
    case hhbc::MemberKey::Tag::ET:
      return HPHP::MemberKey(
        MemberCode::MET,
        toStaticString(mkey.ET._0),
        mkey.ET._1
      );
    case hhbc::MemberKey::Tag::PT: {
      auto const name = toStaticString(mkey.PT._0._0);
      return HPHP::MemberKey(MemberCode::MPT, name, mkey.PT._1);
    }
    case hhbc::MemberKey::Tag::QT: {
      auto const name = toStaticString(mkey.QT._0._0);
      return HPHP::MemberKey(MemberCode::MQT, name, mkey.QT._1);
    }
    case hhbc::MemberKey::Tag::EI:
      return HPHP::MemberKey(MemberCode::MEI, mkey.EI._0, mkey.EI._1);
    case hhbc::MemberKey::Tag::W:
      return HPHP::MemberKey();
  }
  not_reached();
}

ArrayData* TranslationState::getArrayfromAdataId(const AdataId& id) {
  auto const it = adataMap.find(toString(id));
  assertx(it != adataMap.end());
  assertx(it->second->isStatic());
  return it->second;
}

void handleIVA(TranslationState& ts, const uint32_t& iva) {
  ts.fe->emitIVA(iva);
}

void handleIVA(TranslationState& ts, const Local& loc) {
  ts.trackMaxUnnamed(loc.idx);
  ts.fe->emitIVA(loc.idx);
}

void handleLA(TranslationState& ts, const Local& loc) {
  ts.trackMaxUnnamed(loc.idx);
  ts.fe->emitIVA(loc.idx);
}

void handleILA(TranslationState& ts, const Local& loc) {
  ts.trackMaxUnnamed(loc.idx);
  ts.fe->emitIVA(loc.idx);
}

void handleNLA(TranslationState& ts, const Local& local) {
  auto const id = static_cast<int32_t>(local.idx);
  NamedLocal loc{ id, id };
  ts.trackMaxUnnamed(id);
  ts.fe->emitNamedLocal(loc);
}

void handleBLA(TranslationState& ts, const Slice<Label>& labels) {
  ts.fe->emitIVA(labels.len);
  auto targets = range(labels);
  for (auto const& t : targets) {
    ts.addLabelJump(t);
  }
}

template <typename T>
void handleBA(TranslationState& ts, const T& labels) {
  ts.addLabelJump(labels);
}

void handleITA(TranslationState& ts, const hhbc::IterArgs& ita) {
    HPHP::IterArgs ia(
    HPHP::IterArgs::Flags::None,
    ita.iter_id.idx,
    ita.key_id.idx,
    ita.val_id.idx
  );
  ts.trackMaxUnnamed(ita.key_id.idx);
  ts.trackMaxUnnamed(ita.val_id.idx);
  encodeIterArgs(*ts.fe, ia);
}

void handleVSA(TranslationState& ts, const Slice<Str>& arr) {
  ts.fe->emitIVA(arr.len);
  auto strings = range(arr);
  for (auto const& s : strings) {
    auto const str = toStaticString(s);
    ts.fe->emitInt32(ts.mergeLitstr(str));
  }
}

void handleRATA(TranslationState& ts, const hhbc::RepoAuthType& rat) {
  auto str = toStringPiece(rat);
  auto const rat_ = ts.translateRepoAuthType(str);
  encodeRAT(*ts.fe, rat_);
}

void handleKA(TranslationState& ts, const hhbc::MemberKey& mkey) {
  auto const mkey_ = ts.translateMemberKey(mkey);
  encode_member_key(mkey_, *ts.fe);
}

inline bool operator==(const hhbc::Local& a, const hhbc::Local& b) {
  return a.idx == b.idx;
}

inline bool operator==(const hhbc::LocalRange& a, const hhbc::LocalRange& b) {
  return a.start == b.start && a.len == b.len;
}

void handleLAR(TranslationState& ts, const hhbc::LocalRange& lar) {
  auto const lar_ = [&]() {
    if (lar == LocalRange_EMPTY) {
      return HPHP::LocalRange{0, 0};
    }
    auto const firstLoc = lar.start.idx;
    auto const len = lar.len;
    ts.trackMaxUnnamed(firstLoc + len - 1);
    return HPHP::LocalRange{firstLoc, len};
  }();
  encodeLocalRange(*ts.fe, lar_);
}

void handleAA(TranslationState& ts, const AdataId& id) {
  auto const arr = ts.getArrayfromAdataId(id);
  ts.fe->emitInt32(ts.ue->mergeArray(arr));
}

void handleFCA(TranslationState& ts, const hhbc::FCallArgs& fca) {
  auto const internalFlags =
    static_cast<FCallArgsFlags>(fca.flags & HPHP::FCallArgs::kInternalFlags);
  auto const fcaBase = FCallArgsBase(internalFlags, fca.num_args, fca.num_rets);
  auto const async_eager_target = fca.async_eager_target;
  auto const io = fca.inouts;
  auto const readonly = fca.readonly;
  auto const ctx = toStaticString(fca.context);
  auto const numBytes = (fca.num_args + 7) / 8;
  encodeFCallArgs(
    *ts.fe, fcaBase,
    io.len,
    [&] {
      auto inouts = range(io);
      auto result = std::make_unique<uint8_t[]>(numBytes);
      for (int i = 0; i < fca.num_args; i++) {
        result[i / 8] |= inouts[i] << (i % 8);
      }
      for (int i = 0; i < numBytes; i++) {
        ts.fe->emitByte(result[i]);
      }
    },
    readonly.len,
    [&] {
      auto refs = range(readonly);
      auto result = std::make_unique<uint8_t[]>(numBytes);
      for (int i = 0; i < fca.num_args; i++) {
        result[i / 8] |= refs[i] << (i % 8);
      }
      for (int i = 0; i < numBytes; i++) {
        ts.fe->emitByte(result[i]);
      }
    },
    fca.flags & FCallArgsFlags::HasAsyncEagerOffset,
    [&] { ts.addLabelJump(async_eager_target); },
    fca.flags & FCallArgsFlags::ExplicitContext,
    [&] {
      auto const id = ts.mergeLitstr(ctx);
      ts.litstrMap.emplace(id, ctx);
      ts.fe->emitInt32(id);
    }
  );
  if (!fcaBase.hasUnpack()) {
    ts.fe->containsCalls = true;
  }
}

Str getStrfromSA(const ClassName& c) {
  return c._0;
}
Str getStrfromSA(const FunctionName& f) {
  return f._0;
}
Str getStrfromSA(const MethodName& m) {
  return m._0;
}
Str getStrfromSA(const ConstName& c) {
  return c._0;
}
Str getStrfromSA(const PropName& p) {
  return p._0;
}
Str getStrfromSA(const Str& s) {
  return s;
}

template <typename T>
void handleSA(TranslationState& ts, const T& str) {
  auto const sd = toStaticString(getStrfromSA(str));
  auto const id = ts.mergeLitstr(sd);
  ts.fe->emitInt32(id);
}

void handleIA(TranslationState& ts, const IterId& id) {
  ts.fe->emitIVA(id.idx);
}

void handleDA(TranslationState& ts, const FloatBits& fb) {
  ts.fe->emitDouble(fb);
}

void handleI64A(TranslationState& ts, const int64_t& i) {
  ts.fe->emitInt64(i);
}

template<typename T>
void handleOA(TranslationState& ts, T subop) {
  ts.fe->emitByte(static_cast<uint8_t>(subop));
}

//////////////////////////////////////////////////////////////////////

#define IMM_NA
#define IMM_ONE(t) handle##t(ts, body._0);
#define IMM_TWO(t1, t2) IMM_ONE(t1); ++immIdx; handle##t2(ts, body._1)
#define IMM_THREE(t1, t2, t3) IMM_TWO(t1, t2); ++immIdx; handle##t3(ts, body._2)
#define IMM_FOUR(t1, t2, t3, t4) IMM_THREE(t1, t2, t3); ++immIdx; handle##t4(ts, body._3)
#define IMM_FIVE(t1, t2, t3, t4, t5) IMM_FOUR(t1, t2, t3, t4); ++immIdx; handle##t5(ts, body._4)
#define IMM_SIX(t1, t2, t3, t4, t5, t6) IMM_FIVE(t1, t2, t3, t4, t5); ++immIdx; handle##t6(ts, body._5)

#define handleOA(imm) handleOA<imm>
#define handleSLA(ts, imm) do {                           \
  auto const targets = body.targets;                      \
  auto const cases = body.cases;                          \
  ts.fe->emitIVA(cases.len);                              \
  for (int i = 0; i < cases.len-1; i++) {                 \
    auto const caseName = toStaticString(cases.data[i]);  \
    auto const caseId = ts.mergeLitstr(caseName);         \
    ts.fe->emitInt32(caseId);                             \
    ts.addLabelJump(targets.data[i]);                     \
  }                                                       \
  ts.fe->emitInt32(-1);                                   \
  ts.addLabelJump(targets.data[cases.len-1]);             \
} while (0)

#define MAYBE_IMM_ONE(...)
#define MAYBE_IMM_TWO(...)
#define MAYBE_IMM_THREE(...)
#define MAYBE_IMM_FOUR(...)
#define MAYBE_IMM_FIVE(...)
#define MAYBE_IMM_SIX(...)
#define MAYBE_IMM_NA NO_

#define BODY(name) UNUSED auto const body = o.name;
#define NO_BODY(...)

#define GEN_BODY2(id, name) id##BODY(name)
#define GEN_BODY(...) GEN_BODY2(__VA_ARGS__)

#define O(name, imm, pop, push, flags)                                 \
  void translateOpcode##name(TranslationState& ts, const Opcode& o) {  \
    ITRACE(5, "translate Opcode {} \n", #name);                        \
    UNUSED const Offset curOpcodeOff = ts.fe->bcPos();                 \
    ts.fe->emitOp(Op##name);                                           \
                                                                       \
    UNUSED size_t immIdx = 0;                                          \
    GEN_BODY(MAYBE_IMM_##imm, name)                                    \
    IMM_##imm;                                                         \
                                                                       \
    for (auto& kv : ts.labelJumps) {                                   \
      updateLabelSourceforJump(ts, kv.first, kv.second, curOpcodeOff); \
    }                                                                  \
                                                                       \
    /* Record source location. */                                      \
    ts.fe->recordSourceLocation(ts.srcLoc, curOpcodeOff);              \
    ts.labelJumps.clear();                                             \
  }
OPCODES

#undef O
#undef handleSLA
#undef handleOA

typedef void (*translatorFunc)(TranslationState& ts, const Opcode& o);
hphp_hash_map<Opcode::Tag, translatorFunc> opcodeTranslators;

void initializeMap() {
  #define O(name, imm, pop, push, flags) \
    opcodeTranslators[Opcode::Tag::name] = translateOpcode##name;
  OPCODES
  #undef O
}

struct Initializer {
  Initializer() { initializeMap(); }
} initializer;

//////////////////////////////////////////////////////////////////////

void translateOpcodeInstruction(TranslationState& ts, const Opcode& o) {
  return opcodeTranslators[o.tag](ts, o);
}

void translateSrcLoc(TranslationState& ts, const SrcLoc& srcloc) {
  auto const loc = Location::Range(
    srcloc.line_begin,
    srcloc.col_begin,
    srcloc.line_end,
    srcloc.col_end
  );
  ts.srcLoc = loc;
}

void translatePseudoInstruction(TranslationState& ts, const Pseudo& p) {
  switch (p.tag) {
    case Pseudo::Tag::Break:
    case Pseudo::Tag::Continue:
    case Pseudo::Tag::TypedValue:
    case Pseudo::Tag::Comment:
      return;
    case Pseudo::Tag::Label:
      ts.labelMap[p.Label._0].target = ts.fe->bcPos();
      return;
    case Pseudo::Tag::SrcLoc:
      return translateSrcLoc(ts, p.SrcLoc._0);
    // .try {
    case Pseudo::Tag::TryCatchBegin:
      return ts.enterTry();
    // } .catch {
    case Pseudo::Tag::TryCatchMiddle:
      return ts.enterCatch();
    // }
    case Pseudo::Tag::TryCatchEnd:
      return ts.finishTryCatch();
  }
}

void translateInstruction(TranslationState& ts, const Instruct& i) {
  switch (i.tag) {
    case Instruct::Tag::Opcode:
      return translateOpcodeInstruction(ts, i.Opcode._0);
    case Instruct::Tag::Pseudo:
      return translatePseudoInstruction(ts, i.Pseudo._0);
  }
}

void translateDefaultParameterValue(TranslationState& ts,
                                    const Pair<Label,Str>& dv,
                                    FuncEmitter::ParamInfo& param) {
  auto const str = toStaticString(dv._1);
  ts.labelMap[dv._0].dvInits.push_back(ts.fe->params.size());
  parse_default_value(param, str);
}

void upperBoundsHelper(TranslationState& ts,
                       const UpperBoundMap& ubs,
                       const UpperBoundMap& classUbs,
                       const TParamNameVec& shadowedTParams,
                       CompactVector<TypeConstraint>& upperBounds,
                       TypeConstraint& tc,
                       bool hasReifiedGenerics) {
  auto currUBs = getRelevantUpperBounds(tc, ubs, classUbs, shadowedTParams);
  if (currUBs.size() == 1 && !hasReifiedGenerics) {
    applyFlagsToUB(currUBs[0], tc);
    tc = currUBs[0];
  } else if (!currUBs.empty()) {
    upperBounds = std::move(currUBs);
    ts.fe->hasReturnWithMultiUBs = true;
  }
}

template <bool checkPce=false>
bool upperBoundsHelper(TranslationState& ts,
                       const UpperBoundMap& ubs,
                       const UpperBoundMap& classUbs,
                       const TParamNameVec& shadowedTParams,
                       CompactVector<TypeConstraint>& upperBounds,
                       TypeConstraint& tc,
                       const UserAttributeMap& userAttrs) {
  auto const hasReifiedGenerics = [&]() {
    if (userAttrs.find(s___Reified.get()) != userAttrs.end()) return true;
    if (checkPce) {
      return ts.pce->userAttributes().find(s___Reified.get()) !=
             ts.pce->userAttributes().end();
    }
    return false;
  }();

  upperBoundsHelper(ts, ubs, classUbs, shadowedTParams,
                    upperBounds, tc, hasReifiedGenerics);
  return hasReifiedGenerics;
}

void translateParameter(TranslationState& ts,
                        const UpperBoundMap& ubs,
                        const UpperBoundMap& classUbs,
                        const TParamNameVec& shadowedTParams,
                        bool hasReifiedGenerics,
                        const HhasParam& p) {
  FuncEmitter::ParamInfo param;
  translateUserAttributes(p.user_attributes, param.userAttributes);
  if (p.is_variadic) {
    ts.fe->attrs |= AttrVariadicParam;
    param.setFlag(Func::ParamInfo::Flags::Variadic);
  }
  if (p.is_inout) param.setFlag(Func::ParamInfo::Flags::InOut);
  if (p.is_readonly) param.setFlag(Func::ParamInfo::Flags::Readonly);
  std::tie(param.userType, param.typeConstraint)  = maybeOrElse(p.type_info,
      [&](HhasTypeInfo& ti) {return translateTypeInfo(ti);},
      [&]() {return std::make_pair(nullptr, TypeConstraint{});});

  upperBoundsHelper(ts, ubs, classUbs, shadowedTParams, param.upperBounds,
                    param.typeConstraint, hasReifiedGenerics);

  auto const dv = maybe(p.default_value);
  // TODO default value strings are currently escaped
  if (dv) translateDefaultParameterValue(ts, dv.value(), param);
  auto const name = toNamedLocalStaticString(p.name);

  ts.fe->appendParam(name, param);
}

void translateFunctionBody(TranslationState& ts,
                           const HhasBody& b,
                           const UpperBoundMap& ubs,
                           const UpperBoundMap& classUbs,
                           const TParamNameVec& shadowedTParams,
                           bool hasReifiedGenerics) {
  ts.fe->isMemoizeWrapper = b.is_memoize_wrapper | b.is_memoize_wrapper_lsb;
  ts.fe->isMemoizeWrapperLSB = b.is_memoize_wrapper_lsb;
  ts.fe->setNumIterators(b.num_iters);

  auto params = range(b.params);
  for (auto const& p : params) {
    translateParameter(ts, ubs, classUbs, shadowedTParams, hasReifiedGenerics, p);
  }

  auto instrs = range(b.body_instrs);
  for (auto const& instr : instrs) {
    translateInstruction(ts, instr);
  }

  auto const dc = maybe(b.doc_comment);
  if (dc) ts.fe->docComment = makeDocComment(dc.value());

  // Parsing parameters must come before decl_vars
  auto decl_vars = range(b.decl_vars);
  for (auto const& dv : decl_vars) {
    auto const dvName = toNamedLocalStaticString(dv);
    ts.fe->allocVarId(dvName);
  }

  for (auto const& label : ts.labelMap) {
    for (auto const& dvinit : label.second.dvInits) {
      ts.fe->params[dvinit].funcletOff = label.second.target;
    }

    for (auto const& source : label.second.sources) {
      // source.second is the label opcode offset
      // source.first is the offset of where the label is used as an immediate
      ts.fe->emitInt32(label.second.target - source.second, source.first);
    }
  }

  // finish function
  while (ts.fe->numLocals() < ts.maxUnnamed) {
    ts.fe->allocUnnamedLocal();
  }

  ts.fe->finish();
  ts.labelMap.clear();
  assertx(ts.start.empty());
  assertx(ts.handler.empty());
  ts.maxUnnamed = 0;
}

void translateCoeffects(TranslationState& ts, const HhasCoeffects& coeffects) {
  auto static_coeffects = range(coeffects.static_coeffects);
  for (auto const& c : static_coeffects) {
    auto const coeffectStr = CoeffectsConfig::fromHackCCtx(c);
    ts.fe->staticCoeffects.push_back(makeStaticString(coeffectStr));
  }

  auto unenforced_coeffects = range(coeffects.unenforced_static_coeffects);
  for (auto const& c : unenforced_coeffects) {
    ts.fe->staticCoeffects.push_back(toStaticString(c));
  }

  auto fun_param = range(coeffects.fun_param);
  for (auto const& f : fun_param) {
    ts.fe->coeffectRules.emplace_back(
      CoeffectRule(CoeffectRule::FunParam{}, f));
  }

  auto cc_param = range(coeffects.cc_param);
  for (auto const& c : cc_param) {
    ts.fe->coeffectRules.emplace_back(
      CoeffectRule(CoeffectRule::CCParam{}, c._0, toStaticString(c._1)));
  }

  auto cc_this_vec = range(coeffects.cc_this);
  for (auto const& cc_this : cc_this_vec) {

    std::vector<LowStringPtr> names;
    for (int i = 0; i < cc_this.len - 1; i++) {
      names.push_back(toStaticString(cc_this.data[i]));
    }
    auto const ctx_name = toStaticString(cc_this.data[cc_this.len - 1]);
    ts.fe->coeffectRules.emplace_back(
        CoeffectRule(CoeffectRule::CCThis{}, names, ctx_name));
  }

  auto cc_reified_vec = range(coeffects.cc_reified);
  for (auto const& cc_reified : cc_reified_vec) {
    auto const isClass = cc_reified._0;
    auto const pos = cc_reified._1;
    auto types = cc_reified._2;

    std::vector<LowStringPtr> names;
    for (int i = 0; i < types.len - 1; i++) {
      names.push_back(toStaticString(types.data[i]));
    }

    auto const ctx_name = toStaticString(types.data[types.len - 1]);
     ts.fe->coeffectRules.emplace_back(
        CoeffectRule(CoeffectRule::CCReified{}, isClass, pos, names, ctx_name));
  }

  if (coeffects.closure_parent_scope) {
    ts.fe->coeffectRules.emplace_back(CoeffectRule(CoeffectRule::ClosureParentScope{}));
  }

  if (coeffects.generator_this) {
    ts.fe->coeffectRules.emplace_back(CoeffectRule(CoeffectRule::GeneratorThis{}));
  }

  if (coeffects.caller) {
    ts.fe->coeffectRules.emplace_back(CoeffectRule(CoeffectRule::Caller{}));
  }
}

void translateFunction(TranslationState& ts, const HhasFunction& f) {
  UpperBoundMap ubs;
  auto upper_bounds = range(f.body.upper_bounds);
  for (auto const& u : upper_bounds) {
    translateUbs(u, ubs);
  }

  UserAttributeMap userAttrs;
  translateUserAttributes(f.attributes, userAttrs);

  Attr attrs = f.attrs;
  if (!SystemLib::s_inited) {
    attrs |= AttrUnique | AttrPersistent | AttrBuiltin;
  }

  ts.fe = ts.ue->newFuncEmitter(toStaticString(f.name._0));
  ts.fe->init(f.span.line_begin, f.span.line_end, attrs, nullptr);
  ts.fe->isGenerator = (bool)(f.flags & HhasFunctionFlags_GENERATOR);
  ts.fe->isAsync = (bool)(f.flags & HhasFunctionFlags_ASYNC);
  ts.fe->isPairGenerator = (bool)(f.flags & HhasFunctionFlags_PAIR_GENERATOR);
  ts.fe->userAttributes = userAttrs;

  translateCoeffects(ts, f.coeffects);

  auto retTypeInfo = maybeOrElse(f.body.return_type_info,
      [&](HhasTypeInfo& ti) {return translateTypeInfo(ti);},
      [&]() {return std::make_pair(nullptr, TypeConstraint{});});

  auto const hasReifiedGenerics =
    upperBoundsHelper(ts, ubs, {}, {},ts.fe->retUpperBounds, retTypeInfo.second, userAttrs);

  std::tie(ts.fe->retUserType, ts.fe->retTypeConstraint) = retTypeInfo;
  ts.srcLoc = Location::Range{static_cast<int>(f.span.line_begin), -1, static_cast<int>(f.span.line_end), -1};
  translateFunctionBody(ts, f.body, ubs, {}, {}, hasReifiedGenerics);
}

void translateShadowedTParams(TParamNameVec& vec, const Slice<Str>& tpms) {
  auto tparams = range(tpms);
  for (auto const& t : tparams) {
    vec.push_back(toStaticString(t));
  }
}

void translateMethod(TranslationState& ts, const HhasMethod& m, const UpperBoundMap& classUbs) {
  UpperBoundMap ubs;
  auto upper_bounds = range(m.body.upper_bounds);
  for (auto const& u : upper_bounds) {
    translateUbs(u, ubs);
  }

  TParamNameVec shadowedTParams;
  translateShadowedTParams(shadowedTParams, m.body.shadowed_tparams);

  Attr attrs = m.attrs;
  if (!SystemLib::s_inited) attrs |= AttrBuiltin;

  ts.fe = ts.ue->newMethodEmitter(toStaticString(m.name._0), ts.pce);
  ts.pce->addMethod(ts.fe);
  ts.fe->init(m.span.line_begin, m.span.line_end, attrs, nullptr);
  ts.fe->isGenerator = (bool)(m.flags & HhasMethodFlags_IS_GENERATOR);
  ts.fe->isAsync = (bool)(m.flags & HhasMethodFlags_IS_ASYNC);
  ts.fe->isPairGenerator = (bool)(m.flags & HhasMethodFlags_IS_PAIR_GENERATOR);
  ts.fe->isClosureBody = (bool)(m.flags & HhasMethodFlags_IS_CLOSURE_BODY);

  UserAttributeMap userAttrs;
  translateUserAttributes(m.attributes, userAttrs);
  ts.fe->userAttributes = userAttrs;

  translateCoeffects(ts, m.coeffects);

  auto retTypeInfo = maybeOrElse(m.body.return_type_info,
    [&](HhasTypeInfo& ti) {return translateTypeInfo(ti);},
    [&]() {return std::make_pair(nullptr, TypeConstraint{});});

  auto const hasReifiedGenerics =
    upperBoundsHelper<true>(ts, ubs, classUbs, shadowedTParams,
                            ts.fe->retUpperBounds, retTypeInfo.second, userAttrs);

  // TODO(@voork) checkNative
  ts.srcLoc = Location::Range{static_cast<int>(m.span.line_begin), -1,
                              static_cast<int>(m.span.line_end), -1};
  std::tie(ts.fe->retUserType, ts.fe->retTypeConstraint) = retTypeInfo;
  translateFunctionBody(ts, m.body, ubs, classUbs, shadowedTParams, hasReifiedGenerics);
}

void translateClass(TranslationState& ts, const HhasClass& c) {
  UpperBoundMap classUbs;
  auto upper_bounds = range(c.upper_bounds);
  for (auto const& u : upper_bounds) {
    translateUbs(u, classUbs);
  }

  std::string name = toString(c.name._0);
  ITRACE(1, "Translating class {}\n", name);
  ts.pce = ts.ue->newPreClassEmitter(name);

  UserAttributeMap userAttrs;
  ITRACE(2, "Translating attribute list {}\n", c.attributes.len);
  translateUserAttributes(c.attributes, userAttrs);
  auto attrs = c.flags;
  if (!SystemLib::s_inited) attrs |= AttrUnique | AttrPersistent | AttrBuiltin;

  auto const parentName = maybeOrElse(c.base,
    [&](ClassName& s) { return toStaticString(s._0); },
    [&]() { return staticEmptyString(); });

  ts.pce->init(c.span.line_begin,
               c.span.line_end,
               attrs,
               parentName,
               staticEmptyString());

  auto const dc = maybe(c.doc_comment);
  if (dc) ts.pce->setDocComment(makeDocComment(dc.value()));
  ts.pce->setUserAttributes(userAttrs);

  auto impls = range(c.implements);
  for (auto const& i : impls) {
    ts.pce->addInterface(toStaticString(i._0));
  }
  auto incl = range(c.enum_includes);
  for (auto const& in : incl) {
    ts.pce->addEnumInclude(toStaticString(in._0));
  }

  auto requirements = range(c.requirements);
  for (auto const& r : requirements) {
    translateRequirements(ts, r);
  }

  auto methods = range(c.methods);
  for (auto const& m : methods) {
    translateMethod(ts, m, classUbs);
  }

  auto uses = range(c.uses);
  for (auto const& u : uses) {
    ts.pce->addUsedTrait(toStaticString(u._0));
  }

  translateEnumType(ts, c.enum_type);
  translateClassBody(ts, c, classUbs);
}

void translateAdata(TranslationState& ts, const HhasAdata& ad) {
  auto const name = toString(ad.id);
  auto tv = toTypedValue(ad.value);
  auto arr = tv.m_data.parr;
  ArrayData::GetScalarArray(&arr);
  ts.adataMap[name] = arr;
  ts.ue->mergeArray(arr);
}

void translateConstant(TranslationState& ts, const HhasConstant& c) {
  Constant constant;
  constant.name = toStaticString(c.name._0);
  constant.attrs = SystemLib::s_inited ? AttrNone : AttrPersistent;

  constant.val = maybeOrElse(c.value,
    [&](hhbc::TypedValue& tv) {return toTypedValue(tv);},
    [&]() {return make_tv<KindOfNull>();});

  // An uninit constant means its actually a "dynamic" constant whose value
  // is evaluated at runtime. We store the callback in m_data.pcnt and invoke
  // on lookup. (see constant.cpp) It's used for things like STDERR.
  if (type(constant.val) == KindOfUninit) {
    constant.val.m_data.pcnt = reinterpret_cast<MaybeCountable*>(Constant::get);
  }
  ts.ue->addConstant(constant);
}

void translateModuleUse(TranslationState& ts, const Optional<Str>& name) {
  if (!name) return;
  ts.ue->m_moduleName = toStaticString(name.value());
}

void translateModule(TranslationState& ts, const HhasModule& m) {
  UserAttributeMap userAttrs;
  translateUserAttributes(m.attributes, userAttrs);

  ts.ue->addModule(Module{
    toStaticString(m.name._0),
    static_cast<int>(m.span.line_begin),
    static_cast<int>(m.span.line_end),
    Attr(AttrNone),
    userAttrs
  });
}

void translate(TranslationState& ts, const HackCUnit& unit) {
  translateModuleUse(ts, maybe(unit.module_use));
  auto adata = range(unit.adata);
  for (auto const& d : adata) {
    translateAdata(ts, d);
  }

  auto funcs = range(unit.functions);
  for (auto const& f : funcs) {
    translateFunction(ts, f);
  }

  auto classes = range(unit.classes);
  for (auto const& c : classes) {
    translateClass(ts, c);
  }

  auto constants = range(unit.constants);
  for (auto const& c : constants) {
    translateConstant(ts, c);
  }

  auto typedefs = range(unit.typedefs);
  for (auto const& t : typedefs) {
    translateTypedef(ts, t);
  }

  auto modules = range(unit.modules);
  for (auto const& m : modules) {
    translateModule(ts, m);
  }

  translateUserAttributes(unit.file_attributes, ts.ue->m_fileAttributes);
  maybeThen(unit.fatal, [&](Triple<FatalOp, HhasPos, Str> fatal) {
    auto const pos = fatal._1;
    auto const msg = toString(fatal._2);
    throw FatalUnitError(
      msg, ts.ue->m_filepath,
      Location::Range(pos.line_begin, pos.col_begin, pos.line_end, pos.col_end),
      fatal._0
    );
  });

  for (auto& fe : ts.ue->fevec()) fixup_default_values(ts, fe.get());
  for (size_t n = 0; n < ts.ue->numPreClasses(); ++n) {
    for (auto fe : ts.ue->pce(n)->methods()) fixup_default_values(ts, fe);
  }
}
}

std::unique_ptr<UnitEmitter> unitEmitterFromHackCUnit(
  const HackCUnit& unit,
  const char* filename,
	const SHA1& sha1,
	const SHA1& bcSha1,
  const Native::FuncTable& nativeFuncs
) {
  auto ue = std::make_unique<UnitEmitter>(sha1, bcSha1, nativeFuncs);
  StringData* sd = makeStaticString(filename);
  ue->m_filepath = sd;

  try {
    TranslationState ts{};
    ts.ue = ue.get();
    translate(ts, unit);
    ue->finish();
  } catch (const FatalUnitError& e) {
    ue = createFatalUnit(e.filePath, sha1, e.op, e.what(), e.pos);
  }
  return ue;
}
}
