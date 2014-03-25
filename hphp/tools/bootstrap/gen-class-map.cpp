/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "folly/FBString.h"
#include "folly/FBVector.h"

#include "hphp/tools/bootstrap/idl.h"

using folly::fbstring;
using folly::fbvector;
using namespace HPHP::IDL;
using namespace HPHP;

#define VISIBILITY_MASK  (IsPublic|IsProtected|IsPrivate)

/////////////////////////////////////////////////////////////////////////////
// Helpers

static inline fbstring castLong(unsigned long lval, bool hex = false) {
  if (hex) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%lx", lval & 0xffffffff);
    return folly::to<fbstring>("(const char *)0x", buffer);
  }
  return folly::to<fbstring>("(const char*)", lval);
}

/////////////////////////////////////////////////////////////////////////////
// Generated doc comments

static fbstring formatDocComment(const fbstring& comment) {
  const char *p = comment.c_str();
  const char *s;
  const char *e = p + comment.size();
  fbstring ret("/**\n");
  while ((s = strchr(p, '\n'))) {
    ret += " *";
    if ((s - p) > 1) {
      ret += " " + fbstring(p, (s - p));
    }
    ret += "\n";
    p = s + 1;
  }
  if (p < e) {
    ret += " * " + fbstring(p) + "\n";
  }
  return ret + " */";
}

static fbstring genDocCommentPreamble(const fbstring& name,
                                      const fbstring& desc,
                                      long flags,
                                      const fbstring& classname) {
  fbstring ret;

  if (flags & HipHopSpecific) {
    ret = "( HipHop specific )";
  } else {
    ret = "( excerpt from http://php.net/manual/en/";
    if (classname.size()) {
      ret += classname + ".";
    } else {
      ret += "function.";
    }
    auto mangled_name = name;
    for (auto it = mangled_name.begin(); it != mangled_name.end(); ++it) {
      if (*it == '_') {
        *it = '-';
      }
    }
    ret += mangled_name + ".php )";
  }

  if (desc.size()) {
    ret += "\n" + desc;
  }
  return ret + "\n\n";
}

static fbstring genDocComment(const PhpFunc& func,
                              const fbstring& classname) {
  fbstring ret(genDocCommentPreamble(func.getPhpName(), func.getDesc(),
                                     func.flags(), classname));

  for (auto &param : func.params()) {
    ret += "@" + param.name() + " " + param.getPhpType() + " ";
    if (param.isRef()) {
      ret += "(output) ";
    }
    ret += param.getDesc() + "\n";
  }

  if (func.numParams() > 0) {
    ret += "\n";
  }

  auto rko = func.returnKindOf();
  if ((rko != KindOfNull) && (rko != KindOfInvalid)) {
    ret += "@return " + func.returnPhpType() + " ";
    if (func.isReturnRef()) {
      ret += "(output) ";
    }
    ret += func.returnDesc() + "\n";
  }

  return formatDocComment(ret);
}

static fbstring genDocComment(const PhpClass& cls) {
  return formatDocComment(genDocCommentPreamble(cls.getPhpName(),
        cls.getDesc(), cls.flags(), ""));
}

/////////////////////////////////////////////////////////////////////////////
// System constants

static void declareConstants(std::ostream &out,
                             const fbvector<PhpConst>& consts,
                             bool extrn) {
  for (auto c : consts) {
    /*
     * If there's no value, we assume there's an
     * "extern const k_Name = value;" in the c++,
     * plus an "extern const k_Name;" in a header
     * included by ext.h.
     * For the parser tokens T_*, we have the definitions
     * but no header declarations.
     * The exception for KindOfInt64 below is to make sure
     * we declare them in class_map.cpp before using them.
     */
    if (!c.hasValue() && c.kindOf() != KindOfInt64) continue;

    if (extrn || !c.hasValue()) {
      out << "extern const " << c.getCppType() << " " << c.varname() << ";\n";
    } else if (c.kindOf() == KindOfString) {
      fbstring val = c.value();
      out << "extern const StaticString " << c.varname()
          << "(\"" << escapeCpp(val) << "\"," << val.size() << ");\n";
    } else {
      out << "const " << c.getCppType() << " " << c.varname()
          << " = " << escapeCpp(c.value()) << ";\n";
    }
  }
}

static void outputConstants(const char *outputfn,
                            const fbvector<PhpConst>& consts) {
  std::ofstream out(outputfn);

  out << "// @" "generated by gen-class-map.cpp\n"
      << "#ifndef _H_SYSTEM_CONSTANTS\n"
      << "#define _H_SYSTEM_CONSTANTS\n"
      << "namespace HPHP {\n"
      << "class StaticString;\n"
      << "class Variant;\n";
  declareConstants(out, consts, true);
  out << "} // namespace HPHP\n"
      << "#endif // _H_SYSTEM_CONSTANTS\n";
}

/////////////////////////////////////////////////////////////////////////////
// Class Map

#define FUNC_FLAG_MASK (IsProtected|IsPrivate|IsPublic|\
                        IsAbstract|IsStatic|IsFinal|\
                        AllowIntercept|NoProfile|ContextSensitive|\
                        HipHopSpecific|VariableArguments|\
                        RefVariableArguments|MixedVariableArguments|\
                        NoFCallBuiltin|FunctionIsFoldable|\
                        NoInjection|NoEffect|HasOptFunction|ZendParamModeNull|\
                        ZendParamModeFalse|ZendCompat)

static void writeFunction(std::ostream& out, const PhpFunc& func) {
  auto flags = (func.flags() & FUNC_FLAG_MASK) | IsSystem | IsNothing;

  if (flags & RefVariableArguments) {
    flags |= VariableArguments;
  }
  if (flags & MixedVariableArguments) {
    flags |= RefVariableArguments | VariableArguments;
  }
  if (!func.isMethod() || !(flags & VISIBILITY_MASK)) {
    flags |= IsPublic;
  }
  if (func.isReturnRef()) {
    flags |= IsReference;
  }

  out << "  " << castLong(flags, true)
      << ", \"" << escapeCpp(func.getPhpName()) << "\", " << "\"\", "
      << castLong(0) << ", "
      << castLong(0) << ",\n";

  out << "  \""
      << escapeCpp(genDocComment(func, func.className()))
      << "\",\n";

  DataType rko = func.returnKindOf();
  if (rko == KindOfAny) {
    // ClassInfo::MethodInfo expects this for Any/Variant
    // TODO: Fix that broken assumption
    rko = KindOfInvalid;
  }
  out << "  " << castLong(rko, true) << ",\n";
  for (auto &p : func.params()) {
    long attr = IsNothing;
    DataType ko = p.kindOf();
    if (p.isRef()) {
      // We don't declare param type as KindOfRef
      // as then the caller will try to cast it as such
      attr |= IsReference;
      ko = KindOfAny;
    }
    if (ko == KindOfAny) {
      // TODO: See above
      ko = KindOfInvalid;
    }
    out << "  "
        << castLong(attr, true) << ", "
        << "\"" << escapeCpp(p.name()) << "\", \"\", "
        << castLong(ko, true) << ",\n";
    auto ser = p.getDefaultSerialized();
    auto val = p.getDefaultPhp();
    out << "    "
        << "\"" << escapeCpp(ser) << "\", " << castLong(ser.size()) << ", "
        << "\"" << escapeCpp(val) << "\", " << castLong(val.size()) << ", "
        << "NULL,\n";
  }
  out << "  NULL,\n"
      << "  NULL,\n"
      << "  NULL,\n";
}

static void writeConstant(std::ostream& out, const PhpConst& cns) {
  auto name = cns.name();
  out << "  \"" << escapeCpp(name) << "\", ";
  if (cns.hasValue()) {
    auto ser = cns.serialize();
    out << castLong(ser.size())
        << ", \"" << escapeCpp(ser) << "\",\n";
    return;
  }

  if (cns.isSystem()) {
    // Special "magic" constants
    if (name == "SID") {
      out << "(const char *)((offsetof(EnvConstants, k_" << name << ") - "
          << "offsetof(EnvConstants, stgv_Variant)) / sizeof(Variant)), "
          << castLong(1) << ",\n";
      return;
    }
    if ((name == "STDIN") || (name == "STDOUT") || (name == "STDERR")) {
      out << "(const char *)&BuiltinFiles::Get" << name << ", NULL,\n";
      return;
    }
  }

  out << "(const char *)&" << cns.varname() << ", "
      << castLong((int)cns.kindOf() + 2) << ",\n";
}

#define CLASS_FLAG_MASK (IsAbstract|IsFinal|NoDefaultSweep|\
                         HipHopSpecific|IsCppSerializable|ZendCompat)
#define PROP_FLAG_MASK  (IsProtected|IsPrivate|IsPublic|IsStatic)

static void writeClass(std::ostream& out, const PhpClass& cls) {
  auto flags = (cls.flags() & CLASS_FLAG_MASK) | IsSystem | IsNothing;

  out << "  " << castLong(flags, true) << ", "
      << "\"" << escapeCpp(cls.getPhpName()) << "\", "
      << "\"" << escapeCpp(strtolower(cls.parent())) << "\", "
      << "\"\", "
      << castLong(0) << ", "
      << castLong(0) << ",\n";

  out << "  \"" << escapeCpp(genDocComment(cls)) << "\",\n";

  out << "  ";
  for (auto &iface : cls.ifaces()) {
    out << "\"" << escapeCpp(strtolower(iface)) << "\", ";
  }
  out << "NULL,\n";

  for (auto &method : cls.methods()) {
    writeFunction(out, method);
  }
  out << "  NULL,\n";

  for (auto &prop : cls.properties()) {
    auto propflag = (prop.flags() & PROP_FLAG_MASK) | IsNothing;
    if (!(propflag & VISIBILITY_MASK)) {
      propflag |= IsPublic;
    }
    out << "  " << castLong(propflag, true) << ", "
        << "\"" << escapeCpp(prop.name()) << "\", "
        << castLong((int)prop.kindOf(), true) << ",\n";
  }
  out << "  NULL,\n";

  for (auto &cns : cls.constants()) {
    writeConstant(out, cns);
  }
  out << "  NULL,\n";

  out << "  NULL,\n"; // no attributes
}

static void outputClassMap(const char *outputfn, const char *classmap_name,
                           const fbvector<PhpClass>& classes,
                           const fbvector<PhpFunc>& funcs,
                           const fbvector<PhpConst>& consts,
                           const fbvector<PhpExtension>& exts) {
  std::ofstream out(outputfn);

  out << "// @" "generated by gen-class-map.cpp\n"
      << "#include \"hphp/runtime/base/base-includes.h\"\n"
      << "#include \"hphp/runtime/ext/ext.h\"\n"
      << "namespace HPHP {\n";
  declareConstants(out, consts, false);
  out << "const char *" << classmap_name << "[] = {\n";

  for (auto &e : exts) {
    auto sym = e.symbol();
    if (!sym.empty()) {
      out << "  (const char*)&" << sym << ",\n";
    }
  }
  out << "  NULL,\n"; // End of extensions

  out << "  (const char *)ClassInfo::IsSystem, NULL, "
      << "\"\", \"\", NULL, NULL, \"\", NULL,\n";
  for (auto &f : funcs) {
    writeFunction(out, f);
  }
  out << "  NULL,\n"  // End of functions
      << "  NULL,\n"; // End of system "properties"
  for (auto &c : consts) {
    writeConstant(out, c);
  }
  out << "  NULL,\n"  // End of constants
      << "  NULL,\n"; // End of system "attributes"
  for (auto &c : classes) {
    writeClass(out, c);
  }
  out << "  NULL,\n"  // End of classes
      << "  NULL,\n"  // End of classmap
      << "};\n"
      << "} // namespace HPHP\n";
}

void print_usage(const char* program_name) {
  std::cout << "Usage:\n\n"
            << "  " << program_name << "\n"
            << "    --system\n"
            << "    <class map output file>\n"
            << "    <constants output file>\n"
            << "    <*.idl.json>...\n\n"
            << "  " << program_name << "\n"
            << "    <class map name>\n"
            << "    <class map output file>\n"
            << "    <*.idl.json>...\n\n";
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, const char* argv[]) {
  if (argc < 3) {
    print_usage(argv[0]);
    return 0;
  }

  bool system = !strcmp(argv[1], "--system");
  if (system && argc < 4) {
    print_usage(argv[0]);
    return 0;
  }

  fbvector<PhpFunc> funcs;
  fbvector<PhpClass> classes;
  fbvector<PhpConst> consts;
  fbvector<PhpExtension> exts;

  for (int i = (system ? 4 : 3); i < argc; ++i) {
    try {
      parseIDL(argv[i], funcs, classes, consts, exts);
    } catch (const std::exception& exc) {
      std::cerr << argv[i] << ": " << exc.what() << "\n";
      return 1;
    }
  }

  const char* path = argv[2];
  const char* name = (system ? "g_class_map" : argv[1]);
  outputClassMap(path, name, classes, funcs, consts, exts);
  if (system) {
    path = argv[3];
    outputConstants(path, consts);
  }

  return 0;
}
