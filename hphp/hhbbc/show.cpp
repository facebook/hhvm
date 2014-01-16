/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/representation.h"

#include <sstream>
#include <boost/algorithm/string.hpp>

#include "folly/Conv.h"
#include "folly/Format.h"
#include "folly/String.h"
#include "folly/Range.h"
#include "folly/experimental/Gen.h"
#include "folly/experimental/StringGen.h"

#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Pretty printer for debuggin'.
 */

//////////////////////////////////////////////////////////////////////

namespace {

std::string indent(int level, const std::string& s) {
  // Whee; make as many std::string copies as possible.
  auto const space = std::string(level, ' ');
  return space + boost::trim_copy(
    boost::replace_all_copy(s, "\n", "\n" + space)) + "\n";
}

void appendExnTreeString(std::string& ret,
                         borrowed_ptr<const php::ExnNode> p) {
  ret += " " + folly::to<std::string>(p->id);
  if (p->parent) appendExnTreeString(ret, p->parent);
}

std::string escaped_string(SString str) {
  std::string ret;
  if (!str) {
    ret = "<nullptr>";
    return ret;
  }
  auto const sl = str->slice();
  folly::toAppend(
    "\"",
    folly::cEscape<std::string>(
      folly::StringPiece(sl.ptr, sl.len)
    ),
    "\"",
    &ret
  );
  return ret;
};

std::string array_string(SArray arr) {
  std::ostringstream str;
  staticArrayStreamer(const_cast<ArrayData*>(arr), str);
  return str.str();
}

std::string local_string(borrowed_ptr<const php::Local> loc) {
  return loc->name
    ? folly::to<std::string>("$", loc->name->data())
    : folly::to<std::string>("$<unnamed:", loc->id, ">");
};

}

//////////////////////////////////////////////////////////////////////

namespace php {

std::string show(const Block& block) {
  std::string ret;

  if (block.section != php::Block::Section::Main) {
    folly::toAppend("(fault funclet)\n", &ret);
  }

  if (block.exnNode) {
    ret += "(exnNode:";
    appendExnTreeString(ret, block.exnNode);
    ret += ")\n";
  }

  for (auto& bc : block.hhbcs) ret += show(bc) + "\n";

  if (block.fallthrough) {
    folly::toAppend(
      "(fallthrough blk:",
      block.fallthrough->id,
      ")\n",
      &ret
    );
  }
  if (!block.factoredExits.empty()) {
    ret += "(factored exits:";
    for (auto& ex : block.factoredExits) {
      folly::toAppend(" blk:", ex->id, &ret);
    }
    ret += ")\n";
  }

  return ret;
}

std::string show(const Func& func) {
  std::string ret;

  for (auto& blk : func.blocks) {
    ret += folly::format(
      "block #{}\n{}",
      blk->id,
      indent(2, show(*blk))
    ).str();
  }

  visitExnLeaves(func, [&] (const php::ExnNode& node) {
    ret += folly::format("exn node #{} ", node.id).str();
    if (node.parent) {
      ret += folly::format("(^{}) ", node.parent->id).str();
    }
    ret += match<std::string>(
      node.info,
      [&] (const FaultRegion& fr) {
        return folly::to<std::string>("fault->", fr.faultEntry->id);
      },
      [&] (const TryRegion& tr) {
        auto ret = std::string{"catch"};
        using namespace folly::gen;
        from(tr.catches)
          | map([&] (CatchEnt ch) {
              return folly::to<std::string>(ch.first->data(),
                                            "->", ch.second->id);
            })
          | unsplit(" ", &ret)
          ;
        return ret;
      }
    ) + '\n';
  });

  return ret;
}

std::string show(const Class& cls) {
  std::string ret;
  folly::toAppend("class ", cls.name->data(), &ret);
  if (cls.parentName) {
    folly::toAppend(" extends ", cls.parentName->data(), &ret);
  }
  ret += ":\n";
  for (auto& i : cls.interfaceNames) {
    folly::toAppend("  implements ", i->data(), "\n", &ret);
  }
  for (auto& m : cls.methods) {
    folly::toAppend(
      "  method ",
      m->name->data(), ":\n",
      indent(4, show(*m)),
      &ret
    );
  }
  return ret;
}

std::string show(const Unit& unit) {
  std::string ret;
  folly::toAppend(
    "Unit ", unit.filename->data(), "\n",
    "  function pseudomain:\n",
    indent(4, show(*unit.pseudomain)),
    &ret
  );

  for (auto& c : unit.classes) {
    folly::toAppend(
      indent(2, show(*c)),
      &ret
    );
  }

  for (auto& f : unit.funcs) {
    folly::toAppend(
      "  function ", f->name->data(), ":\n",
      indent(4, show(*f)),
      &ret
    );
  }

  folly::toAppend("\n", &ret);
  return ret;
}

std::string show(const Program& p) {
  using namespace folly::gen;
  return from(p.units)
    | map([] (const std::unique_ptr<php::Unit>& u) { return show(*u); })
    | unsplit<std::string>("--------------\n")
    ;
}

std::string show(SrcLoc loc) {
  return folly::format("{}:{}-{}:{}", loc.start.line, loc.start.col,
                                      loc.past.line, loc.past.col).str();
}

}

//////////////////////////////////////////////////////////////////////

std::string show(const Bytecode& bc) {
  std::string ret;

  auto append_mvec = [&] (const MVector& mvec) {
    folly::toAppend(
      "<",
      locationCodeString(mvec.lcode),
      &ret
    );
    if (mvec.locBase) {
      ret += ":" + local_string(mvec.locBase);
    }
    for (auto& m : mvec.mcodes) {
      folly::toAppend(" ", memberCodeString(m.mcode), &ret);
      switch (memberCodeImmType(m.mcode)) {
      case MCodeImm::None: break;
      case MCodeImm::String:
        ret += ":" + escaped_string(m.immStr);
        break;
      case MCodeImm::Int:
        folly::toAppend(":", m.immInt, &ret);
        break;
      case MCodeImm::Local:
        folly::toAppend(":$", m.immLoc->name->data(), &ret);
        break;
      }
    }
    ret += ">";
  };

  auto append_vsa = [&] (const std::vector<SString>& keys) {
    ret += "<";
    auto delim = "";
    for (auto& s : keys) {
      ret += delim + escaped_string(s);
      delim = ",";
    }
    ret += ">";
  };

  auto append_switch = [&] (const SwitchTab& tab) {
    ret += "<";
    for (auto& target : tab) {
      folly::toAppend(target->id, " ", &ret);
    }
    ret += ">";
  };

  auto append_sswitch = [&] (const SSwitchTab& tab) {
    ret += "<";
    for (auto& kv : tab) {
      folly::toAppend(escaped_string(kv.first), ":", kv.second->id, " ", &ret);
    }
    ret += ">";
  };

  auto append_itertab = [&] (const IterTab& tab) {
    ret += "<";
    for (auto& kv : tab) {
      folly::toAppend(kv.first, ",", kv.second->id, " ", &ret);
    }
    ret += ">";
  };

#define IMM_MA(n)      ret += " "; append_mvec(data.mvec);
#define IMM_BLA(n)     ret += " "; append_switch(data.targets);
#define IMM_SLA(n)     ret += " "; append_sswitch(data.targets);
#define IMM_ILA(n)     ret += " "; append_itertab(data.iterTab);
#define IMM_IVA(n)     folly::toAppend(" ", data.arg##n, &ret);
#define IMM_I64A(n)    folly::toAppend(" ", data.arg##n, &ret);
#define IMM_LA(n)      ret += " " + local_string(data.loc##n);
#define IMM_IA(n)      folly::toAppend(" iter:", data.iter##n->id, &ret);
#define IMM_DA(n)      folly::toAppend(" ", data.dbl##n, &ret);
#define IMM_SA(n)      folly::toAppend(" ", escaped_string(data.str##n), &ret);
#define IMM_AA(n)      ret += " " + array_string(data.arr##n);
#define IMM_BA(n)      folly::toAppend(" <blk:", data.target->id, ">", &ret);
#define IMM_OA_IMPL(n) /* empty */
#define IMM_OA(type)   folly::toAppend(" ", subopToName(data.subop), &ret); \
                       IMM_OA_IMPL
#define IMM_VSA(n)     ret += " "; append_vsa(data.keys);

#define IMM_NA
#define IMM_ONE(x)           IMM_##x(1)
#define IMM_TWO(x, y)        IMM_##x(1);         IMM_##y(2);
#define IMM_THREE(x, y, z)   IMM_TWO(x, y);      IMM_##z(3);
#define IMM_FOUR(x, y, z, n) IMM_THREE(x, y, z); IMM_##n(4);

#define O(opcode, imms, inputs, outputs, flags) \
  case Op::opcode:                              \
    {                                           \
      UNUSED auto const& data = bc.opcode;      \
      UNUSED auto const curOpcode = Op::opcode; \
      folly::toAppend(#opcode, &ret);           \
      IMM_##imms                                \
    }                                           \
    break;

  switch (bc.op) { OPCODES }

#undef O

#undef IMM_MA
#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_DA
#undef IMM_SA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA

#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR

  return ret;
}

//////////////////////////////////////////////////////////////////////

std::string show(Type t) {
  std::string ret;

  assert(t.checkInvariants());

  // Unknown union; just bits for now.
  ret = folly::to<std::string>(t.m_bits);

  switch (t.m_bits) {
  case BBottom:      ret = "Bottom";   break;

  case BUninit:      ret = "Uninit";   break;
  case BInitNull:    ret = "InitNull"; break;
  case BFalse:       ret = "False";    break;
  case BTrue:        ret = "True";     break;
  case BInt:         ret = "Int";      break;
  case BDbl:         ret = "Dbl";      break;
  case BSStr:        ret = "SStr";     break;
  case BCStr:        ret = "CStr";     break;
  case BSArr:        ret = "SArr";     break;
  case BCArr:        ret = "CArr";     break;
  case BObj:         ret = "Obj";      break;
  case BRes:         ret = "Res";      break;
  case BCls:         ret = "Cls";      break;
  case BRef:         ret = "Ref";      break;

  case BNull:        ret = "Null";     break;
  case BBool:        ret = "Bool";     break;
  case BStr:         ret = "Str";      break;
  case BArr:         ret = "Arr";      break;

  case BOptTrue:     ret = "?True";    break;
  case BOptFalse:    ret = "?False";   break;
  case BOptInt:      ret = "?Int";     break;
  case BOptDbl:      ret = "?Dbl";     break;
  case BOptBool:     ret = "?Bool";    break;
  case BOptSStr:     ret = "?SStr";    break;
  case BOptCStr:     ret = "?CStr";    break;
  case BOptStr:      ret = "?Str";     break;
  case BOptSArr:     ret = "?SArr";    break;
  case BOptCArr:     ret = "?CArr";    break;
  case BOptArr:      ret = "?Arr";     break;
  case BOptObj:      ret = "?Obj";     break;
  case BOptRes:      ret = "?Res";     break;

  case BInitUnc:     ret = "InitUnc";  break;
  case BUnc:         ret = "Unc";      break;
  case BInitCell:    ret = "InitCell"; break;
  case BCell:        ret = "Cell";     break;
  case BInitGen:     ret = "InitGen";  break;
  case BGen:         ret = "Gen";      break;
  case BTop:         ret = "Top";      break;
  }

  if (t.m_data) {
    if (t.m_bits != BObj && t.m_bits != BCls &&
        t.m_bits != BOptObj) {
      folly::toAppend("=", &ret);
    }

    switch (t.m_bits) {
    case BOptInt:   // fallthrough
    case BInt:      folly::toAppend(t.m_data->ival, &ret); break;
    case BOptDbl:   // fallthrough
    case BDbl:      folly::toAppend(t.m_data->dval, &ret); break;
    case BOptSStr:  // fallthrough
    case BSStr:     ret += escaped_string(t.m_data->sval); break;
    case BOptSArr:  // fallthrough
    case BSArr:     ret += array_string(t.m_data->aval);   break;
    case BOptObj:
      // fallthrough
    case BCls:
      switch (t.m_data->dobj.type) {
      case DObj::Exact:
        folly::toAppend("=", show(t.m_data->dobj.cls), &ret);
        break;
      case DObj::Sub:
        folly::toAppend("<=", show(t.m_data->dobj.cls), &ret);
        break;
      }
      break;
    case BObj:
      switch (t.m_data->dobj.type) {
      case DObj::Exact:
        folly::toAppend("=", show(t.m_data->dobj.cls), &ret);
        break;
      case DObj::Sub:
        folly::toAppend("<=", show(t.m_data->dobj.cls), &ret);
        break;
      }
      break;
    default:
      always_assert(0 && "invalid type with value");
    }
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
