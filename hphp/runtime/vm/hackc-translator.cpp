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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/as.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/hackc-translator.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/zend/zend-string.h"

namespace HPHP {

TRACE_SET_MOD(translate);

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
  UnitEmitter* ue;
  PreClassEmitter* pce{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
// hhbc::Slice helpers
template <class T>
struct Range {
  Slice<T> s;
  const T* begin() { return s.data; }
  const T* end() { return s.data + s.len; }
};

template <class T>
Range<T> mk_range(Slice<T> const& s) {
  return Range<T>{s};
}

///////////////////////////////////////////////////////////////////////////////
// hhbc::Maybe helpers

template<typename T>
Optional<T> maybe(hackc::Maybe<T> m) {
  if (m.tag == hackc::Maybe<T>::Tag::Nothing) return std::nullopt;
  return m.just._0;
}

template<typename T, typename Fn, typename ElseFn>
auto maybeOrElse(hackc::Maybe<T> m, Fn fn, ElseFn efn) {
  auto opt = maybe(m);
  return opt ? fn(opt.value()) : efn();
}

///////////////////////////////////////////////////////////////////////////////
// hhbc::Str Helpers

std::string toString(const Str& str) {
  assertx(str.data != nullptr);
  std::string res {(char*)str.data, 0, str.len};
  return res;
}

StringData* toStaticString(const Str& str) {
  return makeStaticString((char*)str.data, str.len);
}

StringData* makeDocComment(const Str& str) {
  if (RuntimeOption::EvalGenerateDocComments) return toStaticString(str);
  return staticEmptyString();
}

void escapeChar(std::string& res, const Str& str, int& i) {
  auto is_oct = [&] (int i) { return i >= '0' && i <= '7'; };
  auto is_hex = [&] (int i) {
    return (i >= '0' && i <= '9') ||
            (i >= 'a' && i <= 'f') ||
            (i >= 'A' && i <= 'F');
  };
  auto hex_val = [&] (int i) -> uint32_t {
    assertx(is_hex(i));
    return i >= '0' && i <= '9' ? i - '0' :
            i >= 'a' && i <= 'f' ? i - 'a' + 10 : i - 'A' + 10;
  };

  auto c = str.data[++i];

  switch (c) {
  case 'a':  res.push_back('\a'); break;
  case 'b':  res.push_back('\b'); break;
  case 'f':  res.push_back('\f'); break;
  case 'n':  res.push_back('\n'); break;
  case 'r':  res.push_back('\r'); break;
  case 't':  res.push_back('\t'); break;
  case 'v':  res.push_back('\v'); break;
  case '\'': res.push_back('\''); break;
  case '\"': res.push_back('\"'); break;
  case '\?': res.push_back('\?'); break;
  case '\\': res.push_back('\\'); break;
  case '\r': /* ignore */         break;
  case '\n': /* ignore */         break;
  default:
    if (is_oct(c)) {
      auto val = int64_t{c} - '0';
      for (auto j = int{1}; j < 3; ++j) {
        c = str.data[++i];
        if (!is_oct(c)) { i--; break; }
        val *= 8;
        val += c - '0';
      }
      if (val > std::numeric_limits<uint8_t>::max()) {
        error("octal escape sequence overflowed");
      }
      res.push_back(static_cast<uint8_t>(val));
      return;
    }

    if (c == 'x' || c == 'X') {
      auto val = uint64_t{0};
      if (!is_hex(str.data[i+1])) error("\\x used without no following hex digits");
      for (auto j = int{0}; j < 2; ++j) {
        c = str.data[++i];
        if (!is_hex(c)) {i--; break; }
        val *= 0x10;
        val += hex_val(c);
      }
      if (val > std::numeric_limits<uint8_t>::max()) {
        error("hex escape sequence overflowed");
      }
      res.push_back(static_cast<uint8_t>(val));
      return;
    }

    error("unrecognized character escape");
  }
}

StringData* handleQuotedString(const Str& str) {
  if (str.len == 0) return staticEmptyString();
  std::string res;
  int i = 0;
  while (i < str.len) {
    switch (str.data[i]) {
      case '\"': goto finish;
      case '\\': escapeChar(res, str, i); break;
      default  : res.push_back(str.data[i]); break;
    }
    i++;
  }
  finish:
  return makeStaticString(res);
}

///////////////////////////////////////////////////////////////////////////////

using kind = hhbc::TypedValue::Tag;

// TODO make arrays static
HPHP::TypedValue toTypedValue(const hackc::hhbc::TypedValue& tv) {
  switch(tv.tag) {
  case kind::Uninit:
    return make_tv<KindOfUninit>();
  case kind::Int:
    return make_tv<KindOfInt64>(tv.int_._0);
  case kind::Bool:
    return make_tv<KindOfBoolean>(tv.bool_._0);
  case kind::Float: {
    uint8_t buf[8];
    for (int i = 0; i < 8; i++) {
      buf[i] = tv.float_._0._0[7-i];
    }
    double d;
    memcpy(&d, buf, sizeof(buf));
    return make_tv<KindOfDouble>(d);
  }
  case kind::String: {
    auto const s = toStaticString(tv.string._0);
    return make_tv<KindOfPersistentString>(s);
  }
  case kind::Null:
    return make_tv<KindOfNull>();
  case kind::Vec: {
    VecInit v(tv.vec._0.len);
    auto set = mk_range(tv.vec._0);
    for (auto const& elt : set) {
      v.append(toTypedValue(elt));
    }
    return make_tv<KindOfVec>(v.create());
  }
  case kind::Dict: {
    DictInit d(tv.dict._0.len);
    auto set = mk_range(tv.dict._0);
    for (auto const& elt : set) {
        switch (elt._0.tag) {
          case kind::Int:
            d.set(elt._0.int_._0, toTypedValue(elt._1));
            break;
          case kind::String: {
            auto const s = toStaticString(elt._0.string._0);
            d.set(s, toTypedValue(elt._1));
            break;
          }
          default:
            always_assert(false);
        }
    }
    return make_tv<KindOfDict>(d.create());
  }
  case kind::Keyset: {
    KeysetInit k(tv.keyset._0.len);
    auto set = mk_range(tv.keyset._0);
    for (auto const& elt : set) {
      k.add(toTypedValue(elt));
    }
    return make_tv<KindOfKeyset>(k.create());
  }
  case kind::LazyClass: {
    auto const lc = LazyClassData::create(toStaticString(tv.lazy_class._0));
    return make_tv<KindOfLazyClass>(lc);
  }
  case kind::HhasAdata:
    error("toTypedValue unimplemented for HhasAdata");
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
// Field translaters

void translateUserAttributes(Slice<HhasAttribute> attributes, UserAttributeMap& userAttrs) {
  Trace::Indent indent;
  auto attrs = mk_range(attributes);
  for (auto const& attr : attrs) {
    auto const name = handleQuotedString(attr.name);
    VecInit v(attr.arguments.len);
    auto args = mk_range(attr.arguments);
    for (auto const& arg : args) {
      v.append(toTypedValue(arg));
    }
    auto tv = v.create();
    ArrayData::GetScalarArray(&tv);
    userAttrs[name] = make_tv<KindOfVec>(tv);
  };
}

std::pair<const StringData*, TypeConstraint> translateTypeInfo(const HhasTypeInfo& t) {
  auto const user_type = maybeOrElse(t.user_type,
    [&](Str& s) {return toStaticString(s);},
    [&]() {return staticEmptyString();});

  const StringData* type_name = maybeOrElse(t.type_constraint.name,
    [&](Str& s) {return toStaticString(s);},
    [&]() {return nullptr;});

  auto flags = t.type_constraint.flags;
  return std::make_pair(user_type, TypeConstraint{type_name, flags});
}

using TParamNameVec = CompactVector<const StringData*>;
using UpperBoundVec = CompactVector<TypeConstraint>;
using UpperBoundMap = std::unordered_map<const StringData*, UpperBoundVec>;

UpperBoundVec getRelevantUpperBounds(const TypeConstraint& tc,
                                     const UpperBoundMap& ubs,
                                     const UpperBoundMap& class_ubs,
                                     const TParamNameVec& shadowed_tparams) {
  UpperBoundVec ret;
  if (!tc.isTypeVar()) return ret;
  auto const typeName = tc.typeName();
  auto it = ubs.find(typeName);
  if (it != ubs.end()) return it->second;
  if (std::find(shadowed_tparams.begin(), shadowed_tparams.end(), typeName) ==
                shadowed_tparams.end()) {
    it = class_ubs.find(typeName);
    if (it != class_ubs.end()) return it->second;
  }
  return UpperBoundVec{};
}

void translateProperty(TranslationState& ts, const HhasProperty& p, const UpperBoundMap& class_ubs) {
  UserAttributeMap userAttributes;
  translateUserAttributes(p.attributes, userAttributes);

  auto const heredoc = maybeOrElse(p.doc_comment,
    [&](Str& s) {return makeDocComment(s);},
    [&]() {return staticEmptyString();});

  TypeConstraint typeConstraint;
  const StringData* userTy;
  std::tie(userTy, typeConstraint) = translateTypeInfo(p.type_info);

  auto const hasReifiedGenerics =
    userAttributes.find(s___Reified.get()) != userAttributes.end();

  auto ub = getRelevantUpperBounds(typeConstraint, class_ubs, {}, {});

  auto needsMultiUBs = false;
  if (ub.size() == 1 && !hasReifiedGenerics) {
    applyFlagsToUB(ub[0], typeConstraint);
    typeConstraint = ub[0];
  } else if (!ub.empty()) {
    needsMultiUBs = true;
  }

  auto const tv = maybeOrElse(p.initial_value,
    [&](hhbc::TypedValue& s) {return toTypedValue(s);},
    [&]() {return make_tv<KindOfNull>();});

  auto const name = toStaticString(p.name._0);
  ITRACE(2, "Translating property {} {}\n", name, tv.pretty());

  ts.pce->addProperty(
      name,
      p.flags,
      userTy,
      typeConstraint,
      needsMultiUBs ? std::move(ub) : UpperBoundVec{},
      heredoc,
      &tv,
      HPHP::RepoAuthType{},
      userAttributes);
}

void translateClassBody(TranslationState& ts, const HhasClass& c, const UpperBoundMap& class_ubs) {
  auto props = mk_range(c.properties);
  for (auto const& p : props) {
    translateProperty(ts, p, class_ubs);
  }
}

using TypeInfoPair = Pair<Str, Slice<HhasTypeInfo>>;

void translateUbs(const TypeInfoPair& ub, UpperBoundMap& ubs) {
  auto const& name = toStaticString(ub._0);
  CompactVector<TypeConstraint> ret;

  auto infos = mk_range(ub._1);
  for (auto const& i : infos) {
    ubs[name].push_back(translateTypeInfo(i).second);
  }
}

void translateClass(TranslationState& ts, const HhasClass& c) {
  UpperBoundMap ubs;
  auto upper_bounds = mk_range(c.upper_bounds);
  for (auto const& u : upper_bounds) {
    translateUbs(u, ubs);
  }

  std::string name = toString(c.name._0);
  ITRACE(1, "Translating class {}\n", name);
  ts.pce = ts.ue->newPreClassEmitter(name);

  UserAttributeMap userAttrs;
  ITRACE(2, "Translating attribute list {}\n", c.attributes.len);
  translateUserAttributes(c.attributes, userAttrs);
  auto attrs = c.flags;
  if (!SystemLib::s_inited) attrs |= AttrUnique | AttrPersistent | AttrBuiltin;

  auto parentName = maybeOrElse(c.base,
    [&](ClassType& s) { return toStaticString(s._0); },
    [&]() { return staticEmptyString(); });

  ts.pce->init(c.span._0,
               c.span._1,
               attrs,
               parentName,
               staticEmptyString());

  auto impls = mk_range(c.implements);
  for (auto const& i : impls) {
    ts.pce->addInterface(toStaticString(i._0));
  }
  auto incl = mk_range(c.enum_includes);
  for (auto const& in : incl) {
    ts.pce->addEnumInclude(toStaticString(in._0));
  }
  ts.pce->setUserAttributes(userAttrs);
  translateClassBody(ts, c, ubs);
}

void translate(TranslationState& ts, const HhasProgram& prog) {
  auto classes = mk_range(prog.classes);
  for (auto const& c : classes) {
    translateClass(ts, c);
  }
}
}

std::unique_ptr<UnitEmitter> unitEmitterFromHhasProgram(
  const HhasProgram& prog,
  const char* filename,
	const SHA1& sha1,
  const Native::FuncTable& nativeFuncs,
  const std::string& hhasString
) {
  auto const bcSha1 = SHA1{string_sha1(hhasString)};
  auto ue = std::make_unique<UnitEmitter>(sha1, bcSha1, nativeFuncs, false);
  StringData* sd = makeStaticString(filename);
  ue->m_filepath = sd;

  TranslationState ts{};
  ts.ue = ue.get();
  translate(ts, prog);
  ue->finish();
  return ue;
}
}
