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
#include "hphp/runtime/vm/disas.h"

#include <sstream>
#include <map>
#include <queue>
#include <functional>

#include <boost/variant.hpp>

#include <folly/String.h>

#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/type-constraint.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/match.h"

namespace HPHP {

TRACE_SET_MOD(disas);

namespace {

//////////////////////////////////////////////////////////////////////

// Priority queue where the smaller elements come first.
template<class T> using min_priority_queue =
  std::priority_queue<T,std::vector<T>,std::greater<T>>;

//////////////////////////////////////////////////////////////////////

struct Output {
  explicit Output(std::ostream& out)
    : m_out(out)
  {}

  template<class... Args>
  void fmtln(Args&&... args) {
    indent();
    m_out << folly::format(std::forward<Args>(args)...).str()
          << '\n';
  }

  template<class... Args>
  void fmt(Args&&... args) {
    m_out << folly::format(std::forward<Args>(args)...).str();
  }

  void indent() {
    m_out << std::string(m_indentLevel * 2, ' ');
  }

  void nl() { m_out << '\n'; }

  void inc_indent() { ++m_indentLevel; }
  void dec_indent() { --m_indentLevel; }

private:
  uint32_t m_indentLevel = 0;
  std::ostream& m_out;
};

template<class Func>
void indented(Output& out, Func f) {
  out.inc_indent();
  f();
  out.dec_indent();
}

template<class T>
std::string format_line_pair(T* ptr) {
  return folly::sformat(" ({},{})", ptr->line1(), ptr->line2());
}

//////////////////////////////////////////////////////////////////////

std::string escaped(const folly::StringPiece str) {
  return folly::format("\"{}\"", folly::cEscape<std::string>(str)).str();
}

std::string escaped(const StringData* sd) {
  return escaped(sd->slice());
}

/*
 * Python-style long strings.  Any bytes are allowed except three
 * quotes in a row (""").  C-style escapes are also supported.
 *
 * Right now this just takes the C-style escaped version, which can't
 * contain """ bytes anyway, but it's a little worse output than it
 * needs to be.
 */
std::string escaped_long(const StringData* sd) {
  return folly::format("\"\"{}\"\"", escaped(sd)).str();
}

std::string escaped_long(const ArrayData* ad) {
  auto const str = internal_serialize(Variant{const_cast<ArrayData*>(ad)});
  return escaped_long(str.get());
}

std::string escaped_long(TypedValue cell) {
  assertx(tvIsPlausible(cell));
  auto const str = internal_serialize(tvAsCVarRef(&cell));
  return escaped_long(str.get());
}

std::string opt_escaped_long(const StringData* sd) {
  if (!sd || sd->empty()) return {};
  return " " + escaped_long(sd);
}

//////////////////////////////////////////////////////////////////////

struct EHCatchLegacy { std::string label; };
struct EHCatch { Offset end; };
using EHInfo = boost::variant<EHCatchLegacy, EHCatch>;
using UBMap =
  std::unordered_map<const StringData*, std::vector<TypeConstraint>>;
struct FuncInfo {
  FuncInfo(const Unit* u, const Func* f) : unit(u), func(f) {}

  const Unit* unit;
  const Func* func;

  // Map from offset to label names we should use for that offset.
  std::map<Offset,std::string> labels;

  // Information for each EHEnt in the func (basically which label
  // names we chose for its handlers).
  std::unordered_map<const EHEnt*,EHInfo> ehInfo;

  // Try/catch protected region starts in order.
  std::vector<std::pair<Offset,const EHEnt*>> ehStarts;

  // Upper-bounds for params and return types
  UBMap ubs;
};

FuncInfo find_func_info(const Func* func) {
  auto finfo = FuncInfo(func->unit(), func);

  auto label_num = uint32_t{0};
  auto gen_label = [&] (const char* kind) {
    return folly::format("{}{}", kind, label_num++).str();
  };

  auto add_target = [&] (const char* kind, Offset off) -> std::string {
    auto it = finfo.labels.find(off);
    if (it != end(finfo.labels)) return it->second;
    auto const label = gen_label(kind);
    finfo.labels[off] = label;
    return label;
  };

  auto find_jump_targets = [&] {
    auto pc           = func->entry();
    auto const stop   = func->at(func->past());
    auto const bcBase = func->unit()->entry();

    for (; pc != stop; pc += instrLen(pc)) {
      auto const off = func->offsetOf(pc);
      auto const targets = instrJumpTargets(bcBase, off);
      for (auto const& target : targets) {
        add_target("L", target);
      }
    }
  };

  auto find_eh_entries = [&] {
    for (auto& eh : func->ehtab()) {
      finfo.ehInfo[&eh] = [&]() -> EHInfo {
        if (eh.m_end != kInvalidOffset) return EHCatch { eh.m_end };
        return EHCatchLegacy { add_target("C", eh.m_handler) };
      }();
      finfo.ehStarts.emplace_back(eh.m_base, &eh);
    }
  };

  auto find_dv_entries = [&] {
    for (auto i = uint32_t{0}; i < func->numParams(); ++i) {
      auto& param = func->params()[i];
      if (param.hasDefaultValue()) {
        add_target("DV", func->params()[i].funcletOff);
      }
    }
  };

  auto find_upper_bounds = [&] {
    if (func->hasParamsWithMultiUBs()) {
      auto const& params = func->params();
      for (auto const& p : func->paramUBs()) {
        auto const& typeName = params[p.first].typeConstraint.typeName();
        auto& v = finfo.ubs[typeName];
        if (v.empty()) v.assign(std::begin(p.second), std::end(p.second));
      }
    }
    if (func->hasReturnWithMultiUBs()) {
      auto& v = finfo.ubs[func->returnTypeConstraint().typeName()];
      if (v.empty()) {
        v.assign(std::begin(func->returnUBs()), std::end(func->returnUBs()));
      }
    }
  };

  find_jump_targets();
  find_eh_entries();
  find_dv_entries();
  find_upper_bounds();
  return finfo;
}

//////////////////////////////////////////////////////////////////////

template<class T> T decode(PC& pc) {
  auto const ret = *reinterpret_cast<const T*>(pc);
  pc += sizeof ret;
  return ret;
}

std::string loc_name(const FuncInfo& finfo, uint32_t id) {
  auto const sd = finfo.func->localVarName(id);
  if (!sd || sd->empty()) {
    return folly::format("_{}", id).str();
  }
  return folly::format("${}", sd->data()).str();
}

std::string jmp_label(const FuncInfo& finfo, Offset tgt) {
  auto const it  = finfo.labels.find(tgt);
  always_assert(it != end(finfo.labels));
  return it->second;
};

void print_instr(Output& out, const FuncInfo& finfo, PC pc) {
  auto const startPc = pc;

  auto rel_label = [&] (Offset off) {
    auto const tgt = finfo.func->offsetOf(startPc) + off;
    return jmp_label(finfo, tgt);
  };

  auto print_switch = [&] {
    auto const vecLen = decode_iva(pc);
    out.fmt(" <");
    for (auto i = int32_t{0}; i < vecLen; ++i) {
      auto const off = decode<Offset>(pc);
      FTRACE(1, "sw label: {}\n", off);
      out.fmt("{}{}", i != 0 ? " " : "", rel_label(off));
    }
    out.fmt(">");
  };

  auto print_sswitch = [&] {
    auto const vecLen = decode_iva(pc);
    out.fmt(" <");
    for (auto i = int32_t{0}; i < vecLen; ++i) {
      auto const strId  = decode<Id>(pc);
      auto const offset = decode<Offset>(pc);
      out.fmt("{}{}:{}",
        i != 0 ? " " : "",
        strId == -1 ? "-" : escaped(finfo.unit->lookupLitstrId(strId)),
        rel_label(offset)
      );
    }
    out.fmt(">");
  };

  auto print_stringvec = [&] {
    auto const vecLen = decode_iva(pc);
    out.fmt(" <");
    for (auto i = uint32_t{0}; i < vecLen; ++i) {
      auto const str = finfo.unit->lookupLitstrId(decode<int32_t>(pc));
      out.fmt("{}{}", i != 0 ? " " : "", escaped(str));
    }
    out.fmt(">");
  };

  auto print_mk = [&] (MemberKey m) {
    if (m.mcode == MEL || m.mcode == MPL) {
      std::string ret = memberCodeString(m.mcode);
      folly::toAppend(':', loc_name(finfo, m.iva), &ret);
      return ret;
    }
    return show(m);
  };

  // The HHAS format for IterArgs doesn't include flags, so we drop them.
  auto print_ita = [&](const IterArgs& ita) {
    return show(ita, [&](int32_t id) { return loc_name(finfo, id); });
  };

  auto print_fca = [&] (FCallArgs fca) {
    auto const aeLabel = fca.asyncEagerOffset != kInvalidOffset
      ? rel_label(fca.asyncEagerOffset)
      : "-";
    return show(fca, fca.inoutArgs, aeLabel, fca.context);
  };

#define IMM_BLA    print_switch();
#define IMM_SLA    print_sswitch();
#define IMM_IVA    out.fmt(" {}", decode_iva(pc));
#define IMM_I64A   out.fmt(" {}", decode<int64_t>(pc));
#define IMM_LA     out.fmt(" {}", loc_name(finfo, decode_iva(pc)));
#define IMM_NLA    out.fmt(" {}", loc_name(finfo, decode_named_local(pc).id));
#define IMM_ILA    out.fmt(" {}", loc_name(finfo, decode_iva(pc)));
#define IMM_IA     out.fmt(" {}", decode_iva(pc));
#define IMM_DA     out.fmt(" {}", decode<double>(pc));
#define IMM_SA     out.fmt(" {}", \
                           escaped(finfo.unit->lookupLitstrId(decode<Id>(pc))));
#define IMM_RATA   out.fmt(" {}", show(decodeRAT(finfo.unit, pc)));
#define IMM_AA     out.fmt(" @A_{}", decode<Id>(pc));
#define IMM_BA     out.fmt(" {}", rel_label(decode<Offset>(pc)));
#define IMM_OA(ty) out.fmt(" {}", \
                     subopToName(static_cast<ty>(decode<uint8_t>(pc))));
#define IMM_VSA    print_stringvec();
#define IMM_KA     out.fmt(" {}", print_mk(decode_member_key(pc, finfo.unit)));
#define IMM_LAR    out.fmt(" {}", show(decodeLocalRange(pc)));
#define IMM_ITA    out.fmt(" {}", print_ita(decodeIterArgs(pc)));
#define IMM_FCA    out.fmt(" {}", print_fca(decodeFCallArgs(thisOpcode, pc, \
                                                            finfo.unit)));

#define IMM_NA
#define IMM_ONE(x)           IMM_##x
#define IMM_TWO(x,y)         IMM_ONE(x)          IMM_ONE(y)
#define IMM_THREE(x,y,z)     IMM_TWO(x,y)        IMM_ONE(z)
#define IMM_FOUR(x,y,z,l)    IMM_THREE(x,y,z)    IMM_ONE(l)
#define IMM_FIVE(x,y,z,l,m)  IMM_FOUR(x,y,z,l)   IMM_ONE(m)
#define IMM_SIX(x,y,z,l,m,n) IMM_FIVE(x,y,z,l,m) IMM_ONE(n)

  out.indent();
#define O(opcode, imms, ...)                              \
  case Op::opcode: {                                      \
    UNUSED auto const thisOpcode = Op::opcode;            \
    pc += encoded_op_size(Op::opcode);                    \
    out.fmt("{}", #opcode);                               \
    IMM_##imms                                            \
    break;                                                \
  }
  switch (peek_op(pc)) { OPCODES }
#undef O

  assertx(pc == startPc + instrLen(startPc));

#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR
#undef IMM_FIVE
#undef IMM_SIX

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_NLA
#undef IMM_ILA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA
#undef IMM_VSA
#undef IMM_KA
#undef IMM_LAR
#undef IMM_ITA
#undef IMM_FCA

  out.nl();
}

void print_func_directives(Output& out, const FuncInfo& finfo) {
  const Func* func = finfo.func;
  if (RuntimeOption::EvalDisassemblerDocComments) {
    if (func->docComment() && !func->docComment()->empty()) {
      out.fmtln(".doc {};", escaped_long(func->docComment()));
    }
  }
  if (func->isMemoizeWrapper()) {
    out.fmtln(".ismemoizewrapper;");
  }
  if (func->isMemoizeWrapperLSB()) {
    out.fmtln(".ismemoizewrapperlsb;");
  }
  if (auto const niters = func->numIterators()) {
    out.fmtln(".numiters {};", niters);
  }
  if (func->numNamedLocals() > func->numParams()) {
    std::vector<std::string> locals;
    for (int i = func->numParams(); i < func->numNamedLocals(); i++) {
      auto local = loc_name(finfo, i);
      if (!std::all_of(local.begin(), local.end(), is_bareword())) {
        local = escaped(local);
      }
      locals.push_back(local);
    }
    out.fmtln(".declvars {};", folly::join(" ", locals));
  }
}

std::string get_srcloc_str(SourceLoc loc) {
  if (!loc.valid()) return "-1:-1,-1:-1";
  return folly::sformat("{}:{},{}:{}",
                        loc.line0, loc.char0, loc.line1, loc.char1);
}

void print_srcloc(Output& out, SourceLoc loc) {
  out.fmtln(".srcloc {};", get_srcloc_str(loc));
}

void print_func_body(Output& out, const FuncInfo& finfo) {
  auto const func = finfo.func;

  auto       lblIter = begin(finfo.labels);
  auto const lblStop = end(finfo.labels);
  auto       ehIter  = begin(finfo.ehStarts);
  auto const ehStop  = end(finfo.ehStarts);
  auto       bcIter  = func->entry();
  auto const bcStop  = func->at(func->past());

  SourceLoc srcLoc;

  min_priority_queue<Offset> ehEnds;
  min_priority_queue<Offset> ehHandlers;

  while (bcIter != bcStop) {
    auto const off = func->offsetOf(bcIter);

    // First, close any protected EH regions that are past-the-end at
    // this offset.
    while (!ehEnds.empty() && ehEnds.top() == off) {
      ehEnds.pop();
      out.dec_indent();
      out.fmtln("}}");
    }

    if (!ehHandlers.empty() && ehHandlers.top() == off) {
      ehHandlers.pop();
      out.dec_indent();
      out.fmtln("}} .catch {{");
      out.inc_indent();

      // You can't have multiple handlers at the same location.
      assertx(ehHandlers.empty() || ehHandlers.top() != off);
      continue;
    }

    // Next, open any new protected regions that start at this offset.
    for (; ehIter != ehStop && ehIter->first == off; ++ehIter) {
      auto const info = finfo.ehInfo.find(ehIter->second);
      always_assert(info != end(finfo.ehInfo));
      match<void>(
        info->second,
        [&] (const EHCatch& ehCatch) {
          assertx(ehIter->second->m_past == ehIter->second->m_handler);
          out.fmtln(".try {{");
          ehHandlers.push(ehIter->second->m_handler);
          ehEnds.push(ehCatch.end);
        },
        [&] (const EHCatchLegacy& ehCatch) {
          out.fmtln(".try_catch {} {{", ehCatch.label);
          ehEnds.push(ehIter->second->m_past);
        }
      );
      out.inc_indent();
    }

    // Then, print labels if we have any.  This order keeps the labels
    // from dangling on weird sides of .try_fault or .try_catch
    // braces.
    while (lblIter != lblStop && lblIter->first < off) ++lblIter;
    if (lblIter != lblStop && lblIter->first == off) {
      out.dec_indent();
      out.fmtln("{}:", lblIter->second);
      out.inc_indent();
    }

    SourceLoc newLoc;
    finfo.unit->getSourceLoc(off, newLoc);
    if (!(newLoc == srcLoc)) {
      print_srcloc(out, newLoc);
      srcLoc = newLoc;
    }
    print_instr(out, finfo, bcIter);

    bcIter += instrLen(bcIter);
  }
}

std::string type_constraint(const TypeConstraint &tc) {
  std::string typeName = tc.typeName() ? escaped(tc.typeName()) : "N";
  return folly::format("{} {} ",
                       typeName,
                       type_flags_to_string(tc.flags())).str();
}
std::string opt_type_info(const StringData *userType,
                          const TypeConstraint &tc) {
  if (userType || tc.typeName() || tc.flags()) {
    std::string utype = userType ? escaped(userType) : "N";
    return folly::format("<{} {}> ",
                         utype,
                         type_constraint(tc)).str();
  }
  return "";
}

std::string opt_attrs(AttrContext ctx, Attr attrs,
                      const UserAttributeMap* userAttrs = nullptr,
                      bool needPrefix = true) {
  auto str = folly::trimWhitespace(folly::sformat(
               "{} {}",
               attrs_to_string(ctx, attrs), user_attrs(userAttrs))).str();
  if (!str.empty()) {
    str = folly::sformat("{}[{}]", needPrefix ? " " : "", str);
  }
  return str;
}

std::string func_param_list(const FuncInfo& finfo) {
  auto ret = std::string{};
  auto const func = finfo.func;

  for (auto i = uint32_t{0}; i < func->numParams(); ++i) {
    if (i != 0) ret += ", ";

    ret += opt_attrs(AttrContext::Parameter,
        Attr(), &func->params()[i].userAttributes,
        /*needPrefix*/false);

    if (func->params()[i].isVariadic()) {
      ret += "...";
    }
    if (func->isInOut(i)) {
      ret += "inout ";
    }
    ret += opt_type_info(func->params()[i].userType,
                         func->params()[i].typeConstraint);
    ret += folly::format("{}", loc_name(finfo, i)).str();
    if (func->params()[i].hasDefaultValue()) {
      auto const off = func->params()[i].funcletOff;
      ret += folly::format(" = {}", jmp_label(finfo, off)).str();
      if (auto const code = func->params()[i].phpCode) {
        ret += folly::format("({})", escaped_long(code)).str();
      }
    }
  }

  return ret;
}

std::string func_flag_list(const FuncInfo& finfo) {
  auto const func = finfo.func;
  std::vector<std::string> flags;

  if (func->isGenerator()) flags.push_back("isGenerator");
  if (func->isAsync()) flags.push_back("isAsync");
  if (func->isClosureBody()) flags.push_back("isClosureBody");
  if (func->isPairGenerator()) flags.push_back("isPairGenerator");
  if (func->isRxDisabled()) flags.push_back("isRxDisabled");

  std::string strflags = folly::join(" ", flags);
  if (!strflags.empty()) return " " + strflags + " ";
  return " ";
}

// We do not need to emit shadowed type parameters in disassembly
// because we do not emit any type parameters for classes.
// We need to emit the empty list so that assembler can parse it.
std::string opt_shadowed_tparams() {
  return "{}";
}

std::string opt_ubs(const UBMap& ubs) {
  std::string ret = {};
  ret += "{";
  for (auto const& p : ubs) {
    ret += "(";
    ret += p.first->data();
    ret += " as ";
    bool first = true;
    for (auto const& ub : p.second) {
      if (!first) ret += ", ";
      ret += opt_type_info(nullptr, ub);
      first = false;
    }
    ret += ")";
  }
  ret += "}";
  return ret;
}

void print_func(Output& out, const Func* func) {
  auto const finfo = find_func_info(func);

  out.fmtln(".function{}{}{} {}{}({}){}{{",
    opt_ubs(finfo.ubs),
    opt_attrs(AttrContext::Func, func->attrs(), &func->userAttributes()),
    format_line_pair(func),
    opt_type_info(func->returnUserType(), func->returnTypeConstraint()),
    func->name(),
    func_param_list(finfo),
    func_flag_list(finfo));
  indented(out, [&] {
    print_func_directives(out, finfo);
    print_func_body(out, finfo);
  });
  out.fmtln("}}");
  out.nl();
}

std::string member_tv_initializer(TypedValue cell) {
  assertx(tvIsPlausible(cell));
  if (cell.m_type == KindOfUninit) return "uninit";
  return escaped_long(cell);
}

void print_class_constant(Output& out, const PreClass::Const* cns) {
  if (cns->isAbstract()) {
    out.fmtln(".const {}{};", cns->name(),
              cns->isType() ? " isType" : "");
    return;
  }
  out.fmtln(".const {}{} = {};", cns->name(),
    cns->isType() ? " isType" : "",
    member_tv_initializer(cns->val()));
}

template<class T>
void print_prop_or_field_impl(Output& out, const T& f) {
  out.fmtln(".property{}{} {}{} =",
    opt_attrs(AttrContext::Prop, f.attrs(), &f.userAttributes()),
    RuntimeOption::EvalDisassemblerDocComments &&
    RuntimeOption::EvalDisassemblerPropDocComments
      ? opt_escaped_long(f.docComment())
      : std::string(""),
    opt_type_info(f.userType(), f.typeConstraint()),
    f.name()->data());
  indented(out, [&] {
      out.fmtln("{};", member_tv_initializer(f.val()));
  });
}

void print_field(Output& out, const PreRecordDesc::Field& field) {
  print_prop_or_field_impl(out, field);
}

void print_property(Output& out, const PreClass::Prop* prop) {
  print_prop_or_field_impl(out, *prop);
}

void print_method(Output& out, const Func* func) {
  auto const finfo = find_func_info(func);
  out.fmtln(".method{}{}{}{} {}{}({}){}{{",
    opt_shadowed_tparams(),
    opt_ubs(finfo.ubs),
    opt_attrs(AttrContext::Func, func->attrs(), &func->userAttributes()),
    format_line_pair(func),
    opt_type_info(func->returnUserType(), func->returnTypeConstraint()),
    func->name(),
    func_param_list(finfo),
    func_flag_list(finfo));
  indented(out, [&] {
    print_func_directives(out, finfo);
    print_func_body(out, finfo);
  });
  out.fmtln("}}");
}

void print_cls_inheritance_list(Output& out, const PreClass* cls) {
  if (!cls->parent()->empty()) {
    out.fmt(" extends {}", cls->parent());
  }
  if (!cls->interfaces().empty()) {
    out.fmt(" implements (");
    for (auto i = uint32_t{}; i < cls->interfaces().size(); ++i) {
      out.fmt("{}{}", i != 0 ? " " : "", cls->interfaces()[i].get());
    }
    out.fmt(")");
  }
}

void print_cls_enum_ty(Output& out, const PreClass* cls) {
  if (cls->attrs() & AttrEnum) {
    out.fmtln(".enum_ty <{}>;", type_constraint(cls->enumBaseTy()));
  }
}

void print_cls_used_traits(Output& out, const PreClass* cls) {
  auto& traits = cls->usedTraits();
  if (traits.empty()) return;

  out.indent();
  out.fmt(".use");

  for (auto i = uint32_t{0}; i < traits.size(); ++i) {
    out.fmt(" {}", traits[i].get());
  }

  auto& precRules  = cls->traitPrecRules();
  auto& aliasRules = cls->traitAliasRules();
  if (precRules.empty() && aliasRules.empty()) {
    out.fmt(";");
    out.nl();
    return;
  }

  out.fmt(" {{");
  out.nl();
  indented(out, [&] {
    for (auto& prec : precRules) {
      out.fmtln("{}::{} insteadof{};",
        prec.selectedTraitName()->data(),
        prec.methodName()->data(),
        [&]() -> std::string {
          auto ret = std::string{};
          for (auto& name : prec.otherTraitNames()) {
            ret += folly::format(" {}", name.get()).str();
          }
          return ret;
        }()
      );
    }

    for (auto& alias : aliasRules) {
      out.fmtln("{}{} as{}{};",
        alias.traitName()->empty()
          ? std::string{}
          : folly::format("{}::", alias.traitName()).str(),
        alias.origMethodName()->data(),
        opt_attrs(AttrContext::TraitImport, alias.modifiers()),
        alias.newMethodName() != alias.origMethodName()
          ? std::string(" ") + alias.newMethodName()->data()
          : std::string{}
      );
    }
  });
  out.fmtln("}}");
}

void print_requirement(Output& out, const PreClass::ClassRequirement& req) {
  out.fmtln(".require {} <{}>;", req.is_extends() ? "extends" : "implements",
            req.name()->data());
}

void print_cls_directives(Output& out, const PreClass* cls) {
  if (RuntimeOption::EvalDisassemblerDocComments) {
    if (cls->docComment() && !cls->docComment()->empty()) {
      out.fmtln(".doc {};", escaped_long(cls->docComment()));
    }
  }
  print_cls_enum_ty(out, cls);
  print_cls_used_traits(out, cls);
  for (auto& r : cls->requirements())  print_requirement(out, r);
  for (auto& c : cls->allConstants())  print_class_constant(out, &c);
  for (auto& p : cls->allProperties()) print_property(out, &p);
  for (auto* m : cls->allMethods())    print_method(out, m);
}

void print_rec_fields(Output& out, const PreRecordDesc* rec) {
  for (auto& f : rec->allFields()) print_field(out, f);
}

void print_rec(Output& out, const PreRecordDesc* rec) {
  out.indent();
  out.fmt(".record {} {}{}",
      opt_attrs(AttrContext::Class, rec->attrs()),
      rec->name()->toCppString(),
      format_line_pair(rec));
  if (!rec->parentName()->empty()) {
    out.fmt(" extends {}", rec->parentName());
  }
  out.fmt(" {{");
  out.nl();
  if (RuntimeOption::EvalDisassemblerDocComments) {
    if (rec->docComment() && !rec->docComment()->empty()) {
      out.fmtln(".doc {};", escaped_long(rec->docComment()));
    }
  }
  indented(out, [&] { print_rec_fields(out, rec); });
  out.fmt("}}");
  out.nl();
}

void print_cls(Output& out, const PreClass* cls) {
  out.indent();
  auto name = cls->name()->toCppString();
  if (PreClassEmitter::IsAnonymousClassName(name)) {
    auto p = name.find(';');
    if (p != std::string::npos) {
      name = name.substr(0, p);
    }
  }
  UBMap cls_ubs;
  for (auto const& prop : cls->allProperties()) {
    if (prop.upperBounds().empty()) continue;
    auto& v = cls_ubs[prop.typeConstraint().typeName()];
    if (v.empty()) {
      v.assign(std::begin(prop.upperBounds()), std::end(prop.upperBounds()));
    }
  }

  out.fmt(".class {} {} {}{}",
    opt_ubs(cls_ubs),
    opt_attrs(AttrContext::Class, cls->attrs(), &cls->userAttributes()),
    name,
    format_line_pair(cls));
  print_cls_inheritance_list(out, cls);
  out.fmt(" {{");
  out.nl();
  indented(out, [&] { print_cls_directives(out, cls); });
  out.fmtln("}}");
  out.nl();
}

void print_alias(Output& out, const PreTypeAlias& alias) {
  auto flags = TypeConstraint::NoFlags;
  if (alias.nullable) flags = flags | TypeConstraint::Nullable;
  TypeConstraint constraint(alias.value, flags);

  out.fmtln(".alias{} {} = <{}> ({}, {}) {};",
            opt_attrs(AttrContext::Alias, alias.attrs, &alias.userAttrs),
            (const StringData*)alias.name,
            type_constraint(constraint),
            alias.line0,
            alias.line1,
            escaped_long(alias.typeStructure.get()));
}

void print_constant(Output& out, const Constant& cns) {
  out.fmtln(".const{} {} = {};",
            opt_attrs(AttrContext::Constant, cns.attrs),
            cns.name,
            member_tv_initializer(cns.val));
}

void print_fatal(Output& out, const Unit* unit) {
  if (auto const info = unit->getFatalInfo()) {
    out.nl();
    out.fmtln(".fatal {} {} {};",
              get_srcloc_str(SourceLoc{info->m_fatalLoc}),
              subopToName(info->m_fatalOp),
              escaped(info->m_fatalMsg));
  }
}

void print_unit_metadata(Output& out, const Unit* unit) {
  out.nl();

  out.fmtln(".filepath {};", escaped(unit->filepath()));
  print_fatal(out, unit);
  if (!unit->fileAttributes().empty()) {
    out.nl();
    out.fmtln(".file_attributes [{}] ;", user_attrs(&unit->fileAttributes()));
  }
  auto const metaData = unit->metaData();
  for (auto kv : metaData) {
    if (isStringType(kv.second.m_type)) {
      auto isBareWord = true;
      auto isQuoted = true;
      auto const str = kv.second.m_data.pstr;
      for (auto ch : str->slice()) {
        if (!is_bareword()(ch)) isBareWord = false;
        if (ch < ' ' || ch == '"' || ch >= 0x7f) isQuoted = false;
      }
      out.fmtln(".metadata {} = {};",
                kv.first,
                isBareWord ? str->toCppString() :
                isQuoted ? escaped(str) : escaped_long(str));
    }
  }
  for (auto i = size_t{0}; i < unit->numArrays(); ++i) {
    auto const ad = unit->lookupArrayId(i);
    out.fmtln(".adata A_{} = {};", i, escaped_long(ad));
  }
  out.nl();
}

void print_unit(Output& out, const Unit* unit) {
  out.fmtln("# {} starts here", unit->filepath());
  out.nl();
  print_unit_metadata(out, unit);
  for (auto* func : unit->funcs())        print_func(out, func);
  for (auto& cls : unit->preclasses())    print_cls(out, cls.get());
  for (auto& rec : unit->prerecords())    print_rec(out, rec.get());
  for (auto& alias : unit->typeAliases()) print_alias(out, alias);
  for (auto& c : unit->constants())       print_constant(out, c);
  out.fmtln("# {} ends here", unit->filepath());
}

//////////////////////////////////////////////////////////////////////

}

/*
 * Some remaining known issues that aren't supported here (or in
 * conjunction with as.cpp):
 *
 * - .line/.srcpos directives
 */

std::string disassemble(const Unit* unit) {
  std::ostringstream os;
  Output out { os };
  print_unit(out, unit);
  return os.str();
}

std::string user_attrs(const UserAttributeMap* attrsMap) {
  if (!attrsMap || attrsMap->empty()) return "";

  std::vector<std::string> attrs;

  for (auto& entry : *attrsMap) {
    attrs.push_back(
      folly::format("{}({})", escaped(entry.first),
                    escaped_long(entry.second)).str());
  }
  return folly::join(" ", attrs);
}

//////////////////////////////////////////////////////////////////////

}
