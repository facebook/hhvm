/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <boost/variant.hpp>

#include <folly/String.h>

#include "hphp/runtime/base/builtin-functions.h" // f_serialize
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/repo-auth-type-codec.h"
#include "hphp/runtime/vm/as-shared.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/util/match.h"

namespace HPHP {

TRACE_SET_MOD(tmp0);

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

//////////////////////////////////////////////////////////////////////

std::string escaped(const StringData* sd) {
  auto const sl = sd->slice();
  return folly::format("\"{}\"", folly::cEscape<std::string>(sl)).str();
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
  auto const str = f_serialize(Variant{const_cast<ArrayData*>(ad)});
  return escaped_long(str.get());
}

std::string escaped_long(Cell cell) {
  assert(cellIsPlausible(cell));
  auto const str = f_serialize(tvAsCVarRef(&cell));
  return escaped_long(str.get());
}

//////////////////////////////////////////////////////////////////////

struct EHFault { std::string label; };
struct EHCatch { std::map<std::string,std::string> blocks; };
using EHInfo = boost::variant< EHFault
                             , EHCatch
                             >;

struct FuncInfo {
  FuncInfo(const Unit* u, const Func* f) : unit(u), func(f) {}

  const Unit* unit;
  const Func* func;

  // Map from offset to label names we should use for that offset.
  std::map<Offset,std::string> labels;

  // Information for each EHEnt in the func (basically which label
  // names we chose for its handlers).
  std::unordered_map<const EHEnt*,EHInfo> ehInfo;

  // Fault and catch protected region starts in order.
  std::vector<std::pair<Offset,const EHEnt*>> ehStarts;
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
    auto pc           = func->unit()->at(func->base());
    auto const stop   = func->unit()->at(func->past());
    auto const bcBase = func->unit()->at(0);

    for (; pc != stop; pc += instrLen(pc)) {
      auto const op = peek_op(pc);
      auto const off = func->unit()->offsetOf(pc);
      if (isSwitch(op)) {
        foreachSwitchTarget(pc, [&] (Offset off) {
          add_target("L", pc - bcBase + off);
        });
        continue;
      }
      auto const target = instrJumpTarget(bcBase, off);
      if (target != InvalidAbsoluteOffset) {
        add_target("L", target);
        continue;
      }
    }
  };

  auto find_eh_entries = [&] {
    for (auto& eh : func->ehtab()) {
      finfo.ehInfo[&eh] = [&]() -> EHInfo {
        switch (eh.m_type) {
        case EHEnt::Type::Catch:
          {
            auto catches = EHCatch {};
            for (auto& kv : eh.m_catches) {
              auto const clsName = func->unit()->lookupLitstrId(kv.first);
              catches.blocks[clsName->data()] = add_target("C", kv.second);
            }
            return catches;
          }
        case EHEnt::Type::Fault:
          return EHFault { add_target("F", eh.m_fault) };
        }
        not_reached();
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

  find_jump_targets();
  find_eh_entries();
  find_dv_entries();
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
    auto const tgt = startPc - finfo.unit->at(0) + off;
    return jmp_label(finfo, tgt);
  };

  auto print_switch = [&] {
    auto const vecLen = decode<int32_t>(pc);
    out.fmt(" <");
    for (auto i = int32_t{0}; i < vecLen; ++i) {
      auto const off = decode<Offset>(pc);
      FTRACE(1, "sw label: {}\n", off);
      out.fmt("{}{}", i != 0 ? " " : "", rel_label(off));
    }
    out.fmt(">");
  };

  auto print_sswitch = [&] {
    auto const vecLen = decode<int32_t>(pc);
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

  auto print_itertab = [&] {
    auto const vecLen = decode<int32_t>(pc);
    out.fmt(" <");
    for (auto i = int32_t{0}; i < vecLen; ++i) {
      auto const kind = static_cast<IterKind>(decode<int32_t>(pc));
      auto const id   = decode<int32_t>(pc);
      auto const kindStr = [&]() -> const char* {
        switch (kind) {
        case KindOfIter:   return "(Iter)";
        case KindOfMIter:  return "(MIter)";
        case KindOfCIter:  return "(CIter)";
        }
        not_reached();
      }();
      out.fmt("{}{} {}", i != 0 ? ", " : "", kindStr, id);
    }
    out.fmt(">");
  };

  auto print_stringvec = [&] {
    auto const vecLen = decode<int32_t>(pc);
    out.fmt(" <");
    for (auto i = uint32_t{0}; i < vecLen; ++i) {
      auto const str = finfo.unit->lookupLitstrId(decode<int32_t>(pc));
      out.fmt("{}{}", i != 0 ? " " : "", escaped(str));
    }
    out.fmt(">");
  };

#define IMM_BLA    print_switch();
#define IMM_SLA    print_sswitch();
#define IMM_ILA    print_itertab();
#define IMM_IVA    out.fmt(" {}", decodeVariableSizeImm(&pc));
#define IMM_I64A   out.fmt(" {}", decode<int64_t>(pc));
#define IMM_LA     out.fmt(" {}", loc_name(finfo, decodeVariableSizeImm(&pc)));
#define IMM_IA     out.fmt(" {}", decodeVariableSizeImm(&pc));
#define IMM_DA     out.fmt(" {}", decode<double>(pc));
#define IMM_SA     out.fmt(" {}", \
                           escaped(finfo.unit->lookupLitstrId(decode<Id>(pc))));
#define IMM_RATA   out.fmt(" {}", show(decodeRAT(finfo.unit, pc)));
#define IMM_AA     out.fmt(" @A_{}", decode<Id>(pc));
#define IMM_BA     out.fmt(" {}", rel_label(decode<Offset>(pc)));
#define IMM_OA(ty) out.fmt(" {}", \
                     subopToName(static_cast<ty>(decode<uint8_t>(pc))));
#define IMM_VSA    print_stringvec();
#define IMM_KA     out.fmt(" {}", show(decode_member_key(pc, finfo.unit)));

#define IMM_NA
#define IMM_ONE(x)           IMM_##x
#define IMM_TWO(x,y)         IMM_ONE(x)       IMM_ONE(y)
#define IMM_THREE(x,y,z)     IMM_TWO(x,y)     IMM_ONE(z)
#define IMM_FOUR(x,y,z,l)    IMM_THREE(x,y,z) IMM_ONE(l)

  out.indent();
#define O(opcode, imms, ...)                              \
  case Op::opcode:                                        \
    ++pc;                                                 \
    out.fmt("{}", #opcode);                               \
    IMM_##imms                                            \
    break;
  switch (peek_op(pc)) { OPCODES }
#undef O

  assert(pc == startPc + instrLen(startPc));

#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA
#undef IMM_VSA
#undef IMM_KA

  out.nl();
}

void print_func_directives(Output& out, const FuncInfo& finfo) {
  const Func* func = finfo.func;
  if (auto const niters = func->numIterators()) {
    out.fmtln(".numiters {};", niters);
  }
  if (func->numNamedLocals() > func->numParams()) {
    std::vector<std::string> locals;
    for (int i = func->numParams(); i < func->numNamedLocals(); i++) {
      locals.push_back(loc_name(finfo, i));
    }
    out.fmtln(".declvars {};", folly::join(" ", locals));
  }

}

void print_func_body(Output& out, const FuncInfo& finfo) {
  auto const func = finfo.func;

  auto       lblIter = begin(finfo.labels);
  auto const lblStop = end(finfo.labels);
  auto       ehIter  = begin(finfo.ehStarts);
  auto const ehStop  = end(finfo.ehStarts);
  auto       bcIter  = func->unit()->at(func->base());
  auto const bcStop  = func->unit()->at(func->past());

  min_priority_queue<Offset> ehEnds;

  while (bcIter != bcStop) {
    auto const off = func->unit()->offsetOf(bcIter);

    // First, close any protected EH regions that are past-the-end at
    // this offset.
    while (!ehEnds.empty() && ehEnds.top() == off) {
      ehEnds.pop();
      out.dec_indent();
      out.fmtln("}}");
    }

    // Next, open any new protected regions that start at this offset.
    for (; ehIter != ehStop && ehIter->first == off; ++ehIter) {
      auto const info = finfo.ehInfo.find(ehIter->second);
      always_assert(info != end(finfo.ehInfo));
      match<void>(
        info->second,
        [&] (const EHCatch& catches) {
          out.indent();
          out.fmt(".try_catch");
          for (auto& kv : catches.blocks) {
            out.fmt(" ({} {})", kv.first, kv.second);
          }
          out.fmt(" {{");
          out.nl();
        },
        [&] (const EHFault& fault) {
          out.fmtln(".try_fault {} {{", fault.label);
        }
      );
      out.inc_indent();
      ehEnds.push(ehIter->second->m_past);
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

std::string func_param_list(const FuncInfo& finfo) {
  auto ret = std::string{};
  auto const func = finfo.func;

  for (auto i = uint32_t{0}; i < func->numParams(); ++i) {
    if (i != 0) ret += ", ";

    ret += opt_type_info(func->params()[i].userType,
                         func->params()[i].typeConstraint);
    if (func->byRef(i)) ret += "&";
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

  std::string strflags = folly::join(" ", flags);
  if (!strflags.empty()) return " " + strflags + " ";
  return " ";
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

std::string opt_attrs(AttrContext ctx, Attr attrs,
                      const UserAttributeMap* userAttrs = nullptr) {
  auto str = folly::trimWhitespace(folly::format(
               "{} {}",
               attrs_to_string(ctx, attrs), user_attrs(userAttrs)).str()).str();
  if (!str.empty()) str = folly::format(" [{}]", str).str();
  return str;
}

void print_func(Output& out, const Func* func) {
  auto const finfo = find_func_info(func);

  if (func->isPseudoMain()) {
    out.fmtln(".main {{");
  } else {
    out.fmtln(".function{} {}{}({}){}{{",
      opt_attrs(AttrContext::Func, func->attrs(), &func->userAttributes()),
      opt_type_info(func->returnUserType(), func->returnTypeConstraint()),
      func->name(),
      func_param_list(finfo),
      func_flag_list(finfo));
  }
  indented(out, [&] {
    print_func_directives(out, finfo);
    print_func_body(out, finfo);
  });
  out.fmtln("}}");
  out.nl();
}

std::string member_tv_initializer(Cell cell) {
  assert(cellIsPlausible(cell));
  if (cell.m_type == KindOfUninit) return "uninit";
  return escaped_long(cell);
}

void print_constant(Output& out, const PreClass::Const* cns) {
  if (cns->isAbstract()) { return; }
  out.fmtln(".const {}{} = {};", cns->name(),
    cns->isType() ? " isType" : "",
    member_tv_initializer(cns->val()));
}

void print_property(Output& out, const PreClass::Prop* prop) {
  out.fmtln(".property{} {} =",
    opt_attrs(AttrContext::Prop, prop->attrs()),
    prop->name()->data());
  indented(out, [&] {
    out.fmtln("{};", member_tv_initializer(prop->val()));
  });
}

void print_method(Output& out, const Func* func) {
  auto const finfo = find_func_info(func);
  out.fmtln(".method{} {}{}({}){}{{",
    opt_attrs(AttrContext::Func, func->attrs(), &func->userAttributes()),
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

void print_cls_directives(Output& out, const PreClass* cls) {
  print_cls_enum_ty(out, cls);
  print_cls_used_traits(out, cls);
  for (auto& c : cls->allConstants())  print_constant(out, &c);
  for (auto& p : cls->allProperties()) print_property(out, &p);
  for (auto* m : cls->allMethods())    print_method(out, m);
}

void print_cls(Output& out, const PreClass* cls) {
  out.indent();
  out.fmt(".class{} {}",
    opt_attrs(AttrContext::Class, cls->attrs(), &cls->userAttributes()),
    cls->name());
  print_cls_inheritance_list(out, cls);
  out.fmt(" {{");
  out.nl();
  indented(out, [&] { print_cls_directives(out, cls); });
  out.fmtln("}}");
  out.nl();
}

void print_alias(Output& out, const TypeAlias& alias) {
  auto flags = TypeConstraint::NoFlags;
  if (alias.nullable) flags = flags | TypeConstraint::Nullable;
  TypeConstraint constraint(alias.value, flags);

  out.fmtln(".alias{} {} = <{}>;",
            opt_attrs(AttrContext::Alias, alias.attrs),
            (const StringData*)alias.name,
            type_constraint(constraint));
}

void print_unit_metadata(Output& out, const Unit* unit) {
  out.fmtln(".filepath {};", escaped(unit->filepath()));
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
  for (auto& alias : unit->typeAliases()) print_alias(out, alias);
  out.fmtln("# {} ends here", unit->filepath());
}

//////////////////////////////////////////////////////////////////////

}

/*
 * Some remaining known issues that aren't supported here (or in
 * conjunction with as.cpp):
 *
 * - .line/.srcpos directives
 *
 * - Static locals.
 */

std::string disassemble(const Unit* unit) {
  std::ostringstream os;
  Output out { os };
  print_unit(out, unit);
  return os.str();
}

//////////////////////////////////////////////////////////////////////

}
