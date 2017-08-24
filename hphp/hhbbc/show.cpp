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
#include "hphp/hhbbc/representation.h"

#include <sstream>
#include <boost/algorithm/string.hpp>
#include <memory>
#include <vector>

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/String.h>
#include <folly/Range.h>
#include <folly/gen/Base.h>
#include <folly/gen/String.h>

#include "hphp/hhbbc/cfg.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/func-util.h"

#include "hphp/util/text-util.h"

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
  folly::toAppend("\"", folly::cEscape<std::string>(sl), "\"", &ret);
  return ret;
};

std::string array_string(SArray arr) {
  std::string str;
  staticArrayStreamer(const_cast<ArrayData*>(arr), str);
  return str;
}

}

//////////////////////////////////////////////////////////////////////

namespace php {

std::string local_string(const Func& func, LocalId lid) {
  auto const& loc = func.locals[lid];
  return loc.name
    ? folly::to<std::string>("$", loc.name->data())
    : folly::to<std::string>("$<unnamed:", lid, ">");
};

std::string show(const Func& func, const Block& block) {
  std::string ret;

  if (block.section != php::Block::Section::Main) {
    folly::toAppend("(fault funclet)\n", &ret);
  }

  if (block.exnNode) {
    ret += "(exnNode:";
    appendExnTreeString(ret, block.exnNode);
    ret += ")\n";
  }

  for (auto& bc : block.hhbcs) ret += show(func, bc) + "\n";

  if (block.fallthrough != NoBlockId) {
    folly::toAppend(
      "(fallthrough blk:",
      block.fallthrough,
      ")\n",
      &ret
    );
  }
  if (!block.factoredExits.empty()) {
    ret += "(factored exits:";
    for (auto ex : block.factoredExits) {
      folly::toAppend(" blk:", ex, &ret);
    }
    ret += ")\n";
  }

  return ret;
}

std::string dot_instructions(const Func& func, const Block& b) {
  using namespace folly::gen;
  return from(b.hhbcs)
    | map([&] (const Bytecode& bc) {
        return "\"" + folly::cEscape<std::string>(show(func, bc)) + "\\n\"";
      })
    | unsplit<std::string>("+\n")
    ;
}

// Output DOT-format graph.  Paste into dot -Txlib or similar.
std::string dot_cfg(const Func& func) {
  std::string ret;
  for (auto& b : rpoSortAddDVs(func)) {
    ret += folly::format(
      "B{} [ label = \"blk:{}\\n\"+{} ]\n",
      b->id, b->id, dot_instructions(func, *b)).str();
    bool outputed = false;
    forEachNormalSuccessor(*b, [&] (BlockId target) {
      ret += folly::format("B{} -> B{};", b->id, target).str();
      outputed = true;
    });
    if (outputed) ret += "\n";
    outputed = false;
    if (!is_single_nop(*b)) {
      for (auto ex : b->factoredExits) {
        ret += folly::sformat("B{} -> B{} [color=blue];", b->id, ex);
        outputed = true;
      }
    }
    if (outputed) ret += "\n";
  }
  return ret;
}

std::string show(const Func& func) {
  std::string ret;

#define X(what) if (func.what) folly::toAppend(#what "\n", &ret)
  X(isClosureBody);
  X(isAsync);
  X(isGenerator);
  X(isPairGenerator);
#undef X

  if (getenv("HHBBC_DUMP_DOT")) {
    folly::format(&ret,
                  "digraph {} {{\n  node [shape=box];\n{}}}\n",
                  func.name, indent(2, dot_cfg(func)));
  }

  for (auto& blk : func.blocks) {
    if (blk->id == NoBlockId) continue;
    folly::format(&ret, "block #{} (section {})\n{}",
                  blk->id, static_cast<size_t>(blk->section),
                  indent(2, show(func, *blk)));
  }

  visitExnLeaves(func, [&] (const php::ExnNode& node) {
    folly::format(&ret, "exn node #{} ", node.id);
    if (node.parent) {
      folly::format(&ret, "(^{}) ", node.parent->id);
    }
    ret += match<std::string>(
      node.info,
      [&] (const FaultRegion& fr) {
        return folly::to<std::string>("fault->", fr.faultEntry);
      },
      [&] (const CatchRegion& cr) {
        return folly::to<std::string>("catch->", cr.catchEntry);
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
  return folly::sformat("{}:{}-{}:{}",
                        loc.start.line, loc.start.col,
                        loc.past.line, loc.past.col);
}

}

//////////////////////////////////////////////////////////////////////

std::string show(const php::Func& func, const Bytecode& bc) {
  std::string ret;

  auto append_vsa = [&] (const CompactVector<LSString>& keys) {
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
      folly::toAppend(target, " ", &ret);
    }
    ret += ">";
  };

  auto append_sswitch = [&] (const SSwitchTab& tab) {
    ret += "<";
    for (auto& kv : tab) {
      folly::toAppend(escaped_string(kv.first), ":", kv.second, " ", &ret);
    }
    ret += ">";
  };

  auto append_itertab = [&] (const IterTab& tab) {
    ret += "<";
    for (auto& kv : tab) {
      folly::toAppend(kv.first, ",", kv.second, " ", &ret);
    }
    ret += ">";
  };

  auto append_mkey = [&](MKey mkey) {
    ret += memberCodeString(mkey.mcode);

    switch (mkey.mcode) {
      case MEL: case MPL:
        folly::toAppend(':', local_string(func, mkey.local), &ret);
        break;
      case MEC: case MPC:
        folly::toAppend(':', mkey.idx, &ret);
        break;
      case MEI:
        folly::toAppend(':', mkey.int64, &ret);
        break;
      case MET: case MPT: case MQT:
        folly::toAppend(
          ":\"", escapeStringForCPP(mkey.litstr->data(), mkey.litstr->size()),
          '"', &ret
        );
        break;
      case MW:
        break;
    }
  };

  auto append_lar = [&](const LocalRange& range) {
    folly::toAppend("L:", local_string(func, range.first), "+",
                    range.restCount, &ret);
  };

#define IMM_BLA(n)     ret += " "; append_switch(data.targets);
#define IMM_SLA(n)     ret += " "; append_sswitch(data.targets);
#define IMM_ILA(n)     ret += " "; append_itertab(data.iterTab);
#define IMM_IVA(n)     folly::toAppend(" ", data.arg##n, &ret);
#define IMM_I64A(n)    folly::toAppend(" ", data.arg##n, &ret);
#define IMM_LA(n)      ret += " " + local_string(func, data.loc##n);
#define IMM_IA(n)      folly::toAppend(" iter:", data.iter##n, &ret);
#define IMM_CAR(n)     folly::toAppend(" rslot:", data.slot, &ret);
#define IMM_CAW(n)     folly::toAppend(" wslot:", data.slot, &ret);
#define IMM_DA(n)      folly::toAppend(" ", data.dbl##n, &ret);
#define IMM_SA(n)      folly::toAppend(" ", escaped_string(data.str##n), &ret);
#define IMM_RATA(n)    folly::toAppend(" ", show(data.rat), &ret);
#define IMM_AA(n)      ret += " " + array_string(data.arr##n);
#define IMM_BA(n)      folly::toAppend(" <blk:", data.target, ">", &ret);
#define IMM_OA_IMPL(n) folly::toAppend(" ", subopToName(data.subop##n), &ret);
#define IMM_OA(type)   IMM_OA_IMPL
#define IMM_VSA(n)     ret += " "; append_vsa(data.keys);
#define IMM_KA(n)      ret += " "; append_mkey(data.mkey);
#define IMM_LAR(n)     ret += " "; append_lar(data.locrange);

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

#undef IMM_BLA
#undef IMM_SLA
#undef IMM_ILA
#undef IMM_IVA
#undef IMM_I64A
#undef IMM_LA
#undef IMM_IA
#undef IMM_CAR
#undef IMM_CAW
#undef IMM_DA
#undef IMM_SA
#undef IMM_RATA
#undef IMM_AA
#undef IMM_BA
#undef IMM_OA_IMPL
#undef IMM_OA
#undef IMM_KA
#undef IMM_LAR

#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR

  return ret;
}

//////////////////////////////////////////////////////////////////////

std::string show(const Type& t) {
  std::string ret;

  assert(t.checkInvariants());

  if (is_specialized_wait_handle(t)) {
    folly::format(&ret, "{}WaitH<{}>",
                  is_opt(t) ? "?" : "", show(wait_handle_inner(t)));
    return ret;
  }

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
  case BSArrE:       ret = "SArrE";    break;
  case BCArrE:       ret = "CArrE";    break;
  case BSArrN:       ret = "SArrN";    break;
  case BCArrN:       ret = "CArrN";    break;
  case BArrE:        ret = "ArrE";     break;
  case BArrN:        ret = "ArrN";     break;
  case BObj:         ret = "Obj";      break;
  case BRes:         ret = "Res";      break;
  case BCls:         ret = "Cls";      break;
  case BRef:         ret = "Ref";      break;

  case BSVec:        ret = "SVec";     break;
  case BCVec:        ret = "CVec";     break;
  case BSVecE:       ret = "SVecE";    break;
  case BCVecE:       ret = "CVecE";    break;
  case BSVecN:       ret = "SVecN";    break;
  case BCVecN:       ret = "CVecN";    break;
  case BVecE:        ret = "VecE";     break;
  case BVecN:        ret = "VecN";     break;
  case BSDict:       ret = "SDict";    break;
  case BCDict:       ret = "CDict";    break;
  case BSDictE:      ret = "SDictE";   break;
  case BCDictE:      ret = "CDictE";   break;
  case BSDictN:      ret = "SDictN";   break;
  case BCDictN:      ret = "CDictN";   break;
  case BDictE:       ret = "DictE";    break;
  case BDictN:       ret = "DictN";    break;
  case BSKeyset:     ret = "SKeyset";  break;
  case BCKeyset:     ret = "CKeyset";  break;
  case BSKeysetE:    ret = "SKeysetE"; break;
  case BCKeysetE:    ret = "CKeysetE"; break;
  case BSKeysetN:    ret = "SKeysetN"; break;
  case BCKeysetN:    ret = "CKeysetN"; break;
  case BKeysetE:     ret = "KeysetE";  break;
  case BKeysetN:     ret = "KeysetN";  break;

  case BNull:        ret = "Null";     break;
  case BNum:         ret = "Num";      break;
  case BBool:        ret = "Bool";     break;
  case BStr:         ret = "Str";      break;
  case BArr:         ret = "Arr";      break;
  case BVec:         ret = "Vec";      break;
  case BDict:        ret = "Dict";     break;
  case BKeyset:      ret = "Keyset";   break;

  case BOptTrue:     ret = "?True";    break;
  case BOptFalse:    ret = "?False";   break;
  case BOptInt:      ret = "?Int";     break;
  case BOptDbl:      ret = "?Dbl";     break;
  case BOptNum:      ret = "?Num";     break;
  case BOptBool:     ret = "?Bool";    break;
  case BOptSStr:     ret = "?SStr";    break;
  case BOptCStr:     ret = "?CStr";    break;
  case BOptStr:      ret = "?Str";     break;
  case BOptSArrE:    ret = "?SArrE";   break;
  case BOptCArrE:    ret = "?CArrE";   break;
  case BOptSArrN:    ret = "?SArrN";   break;
  case BOptCArrN:    ret = "?CArrN";   break;
  case BOptSArr:     ret = "?SArr";    break;
  case BOptCArr:     ret = "?CArr";    break;
  case BOptArrE:     ret = "?ArrE";    break;
  case BOptArrN:     ret = "?ArrN";    break;
  case BOptArr:      ret = "?Arr";     break;
  case BOptObj:      ret = "?Obj";     break;
  case BOptRes:      ret = "?Res";     break;

  case BOptSVecE:    ret = "?SVecE";   break;
  case BOptCVecE:    ret = "?CVecE";   break;
  case BOptSVecN:    ret = "?SVecN";   break;
  case BOptCVecN:    ret = "?CVecN";   break;
  case BOptSVec:     ret = "?SVec";    break;
  case BOptCVec:     ret = "?CVec";    break;
  case BOptVecE:     ret = "?VecE";    break;
  case BOptVecN:     ret = "?VecN";    break;
  case BOptVec:      ret = "?Vec";     break;
  case BOptSDictE:   ret = "?SDictE";  break;
  case BOptCDictE:   ret = "?CDictE";  break;
  case BOptSDictN:   ret = "?SDictN";  break;
  case BOptCDictN:   ret = "?CDictN";  break;
  case BOptSDict:    ret = "?SDict";   break;
  case BOptCDict:    ret = "?CDict";   break;
  case BOptDictE:    ret = "?DictE";   break;
  case BOptDictN:    ret = "?DictN";   break;
  case BOptDict:     ret = "?Dict";    break;
  case BOptSKeysetE: ret = "?SKeysetE";break;
  case BOptCKeysetE: ret = "?CKeysetE";break;
  case BOptSKeysetN: ret = "?SKeysetN";break;
  case BOptCKeysetN: ret = "?CKeysetN";break;
  case BOptSKeyset:  ret = "?SKeyset"; break;
  case BOptCKeyset:  ret = "?CKeyset"; break;
  case BOptKeysetE:  ret = "?KeysetE"; break;
  case BOptKeysetN:  ret = "?KeysetN"; break;
  case BOptKeyset:   ret = "?Keyset";  break;

  case BUncArrKey:    ret = "UncArrKey";  break;
  case BArrKey:       ret = "ArrKey";     break;
  case BOptUncArrKey: ret = "?UncArrKey"; break;
  case BOptArrKey:    ret = "?ArrKey";    break;

  case BInitPrim:    ret = "InitPrim"; break;
  case BPrim:        ret = "Prim";     break;
  case BInitUnc:     ret = "InitUnc";  break;
  case BUnc:         ret = "Unc";      break;
  case BInitCell:    ret = "InitCell"; break;
  case BCell:        ret = "Cell";     break;
  case BInitGen:     ret = "InitGen";  break;
  case BGen:         ret = "Gen";      break;
  case BTop:         ret = "Top";      break;
  }

  switch (t.m_dataTag) {
  case DataTag::None:
    return ret;
  case DataTag::Obj:
  case DataTag::Cls:
  case DataTag::RefInner:
  case DataTag::ArrLikePacked:
  case DataTag::ArrLikePackedN:
  case DataTag::ArrLikeMap:
  case DataTag::ArrLikeMapN:
    break;
  case DataTag::ArrLikeVal:
    folly::toAppend("~", &ret);
    break;
  case DataTag::Str:
  case DataTag::Int:
  case DataTag::Dbl:
    folly::toAppend("=", &ret);
    break;
  }

  auto showElem = [&] (const Type& key, const Type& val) -> std::string {
    if (t.subtypeOf(TOptKeyset)) return show(key);
    return show(key) + ":" + show(val);
  };

  switch (t.m_dataTag) {
  case DataTag::Int: folly::toAppend(t.m_data.ival, &ret); break;
  case DataTag::Dbl: folly::toAppend(t.m_data.dval, &ret); break;
  case DataTag::Str: ret += escaped_string(t.m_data.sval); break;
  case DataTag::ArrLikeVal:
    ret += array_string(t.m_data.aval);
    break;
  case DataTag::Obj:
    switch (t.m_data.dobj.type) {
    case DObj::Exact:
      folly::toAppend("=", show(t.m_data.dobj.cls), &ret);
      break;
    case DObj::Sub:
      folly::toAppend("<=", show(t.m_data.dobj.cls), &ret);
      break;
    }
    break;
  case DataTag::Cls:
    switch (t.m_data.dcls.type) {
    case DCls::Exact:
      folly::toAppend("=", show(t.m_data.dcls.cls), &ret);
      break;
    case DCls::Sub:
      folly::toAppend("<=", show(t.m_data.dcls.cls), &ret);
      break;
    }
    break;
  case DataTag::RefInner:
    folly::toAppend("(", show(*t.m_data.inner), ")", &ret);
    break;
  case DataTag::None:
    break;
  case DataTag::ArrLikePacked:
    folly::format(
      &ret,
      "({})",
      [&] {
        using namespace folly::gen;
        return from(t.m_data.packed->elems)
          | map([&] (const Type& t) { return show(t); })
          | unsplit<std::string>(",");
      }()
    );
    break;
  case DataTag::ArrLikePackedN:
    folly::format(&ret, "([{}])", show(t.m_data.packedn->type));
    break;
  case DataTag::ArrLikeMap:
    folly::format(
      &ret,
      "({})",
      [&] {
        using namespace folly::gen;
        return from(t.m_data.map->map)
          | map([&] (const std::pair<Cell,Type>& kv) {
              return showElem(from_cell(kv.first), kv.second);
            })
          | unsplit<std::string>(",");
      }()
    );
    break;
  case DataTag::ArrLikeMapN:
    folly::format(
      &ret,
      "([{}])",
      showElem(t.m_data.mapn->key, t.m_data.mapn->val)
    );
    break;
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

std::string show(Context ctx) {
  auto ret = std::string{};
  if (is_pseudomain(ctx.func)) {
    ret = ctx.func->unit->filename->data();
    ret += ";pseudomain";
    return ret;
  }
  if (ctx.cls) {
    ret = ctx.cls->name->data();
    ret += "::";
  }
  ret += ctx.func->name->data();
  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
