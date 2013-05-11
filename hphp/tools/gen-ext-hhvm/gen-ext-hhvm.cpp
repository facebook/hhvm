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

#include <algorithm>
#include <cxxabi.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "folly/FBString.h"
#include "folly/Format.h"
#include "folly/ScopeGuard.h"

#include "idl.h"

using folly::fbstring;

std::unordered_map<fbstring, const PhpFunc*> g_mangleMap;
std::unordered_map<fbstring, const PhpClass*> g_classMap;

// Functions with return types that don't fit in registers are handled
// differently on ARM. Instead of using a pointer-to-return-value-space hidden
// first parameter like on x64, the pointer is passed in a register reserved for
// this purpose, not part of the normal argument sequence.
bool g_armMode = false;

constexpr char* g_allIncludes = R"(
#include "runtime/ext_hhvm/ext_hhvm.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/base/array/array_init.h"
#include "runtime/ext/ext.h"
#include "runtime/vm/class.h"
#include "runtime/vm/runtime.h"
#include <exception>
)";

///////////////////////////////////////////////////////////////////////////////
// Code emission helpers -- leaf functions

void emitCtorHelper(const fbstring& className, std::ostream& out) {
  out << folly::format(
    R"(
HPHP::VM::Instance* new_{0:s}_Instance(HPHP::VM::Class* cls) {{
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_{0:s}) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_{0:s}(cls);
  return inst;
}})",
    className) << "\n\n";
}

/*
 * Emits a declaration that corresponds to the f_* functions, but with types and
 * signatures adjusted to reflect the underlying C++ ABI.
 */
void emitRemappedFuncDecl(const PhpFunc& func,
                          const fbstring& mangled,
                          const fbstring& prefix,
                          std::ostream& out) {
  if (isTypeCppIndirectPass(func.returnCppType)) {
    if (func.returnCppType == "HPHP::Variant") {
      out << "TypedValue* ";
    } else {
      out << "Value* ";
    }
  } else {
    out << func.returnCppType << ' ';
  }

  out << prefix << func.getUniqueName() << '(';

  bool isFirstParam = true;

  if (!g_armMode && isTypeCppIndirectPass(func.returnCppType)) {
    if (func.returnCppType == "HPHP::Variant") {
      out << "TypedValue* _rv";
    } else {
      out << "Value* _rv";
    }
    isFirstParam = false;
  }

  if (!func.className.empty() && !func.isStatic) {
    if (!isFirstParam) {
      out << ", ";
    }
    out << "ObjectData* this_";
    isFirstParam = false;
  }

  if (func.isVarargs) {
    if (!isFirstParam) {
      out << ", ";
    }
    out << "int64_t _argc";
    isFirstParam = false;
  }

  for (auto const& param : func.params) {
    if (!isFirstParam) {
      out << ", ";
    }
    if (isTypeCppIndirectPass(param.cppType)) {
      if (param.cppType == "HPHP::Variant" ||
          param.cppType == "HPHP::Variant const&" ||
          param.cppType == "HPHP::VRefParamValue const&") {
        out << "TypedValue*";
      } else {
        out << "Value*";
      }
    } else {
      out << param.cppType;
    }
    out << ' ' << param.name;
    isFirstParam = false;
  }

  if (func.isVarargs) {
    assert(!isFirstParam);
    out << ", Value* _argv";
  }

  out << ") asm(\""
      << mangled << "\");\n\n";
}


void emitCast(const PhpParam& param, int32_t index, std::ostream& out,
              const char* ind, bool doCheck) {
  if (doCheck) {
    out << ind << "if (";
    if (param.cppType == "HPHP::String const&") {
      out << "!IS_STRING_TYPE((args-" << index << ")->m_type)";
    } else {
      out << "(args-" << index << ")->m_type != ";
      if (param.cppType == "bool") {
        out << "KindOfBoolean";
      } else if (param.cppType == "int" || param.cppType == "long") {
        out << "KindOfInt64";
      } else if (param.cppType == "double") {
        out << "KindOfDouble";
      } else if (param.cppType == "HPHP::Array const&") {
        out << "KindOfArray";
      } else if (param.cppType == "HPHP::Object const&") {
        out << "KindOfObject";
      }
    }
    out << ") {\n";
    ind -= 2;
  }

  out << ind << "tvCastTo";
  if (param.cppType == "bool") {
    out << "Boolean";
  } else if (param.cppType == "int" || param.cppType == "long") {
    out << "Int64";
  } else if (param.cppType == "double") {
    out << "Double";
  } else if (param.cppType == "HPHP::String const&") {
    out << "String";
  } else if (param.cppType == "HPHP::Array const&") {
    out << "Array";
  } else if (param.cppType == "HPHP::Object const&") {
    out << "Object";
  }
  out << "InPlace(args-" << index << ");\n";

  if (doCheck) {
    ind += 2;
    out << ind << "}\n";
  }
}


/*
 * Emits an expression which will check the types of arguments on the VM stack.
 */
void emitTypechecks(const PhpFunc& func, std::ostream& out, const char* ind) {
  bool isFirstParam = true;
  for (int k = func.maxNumParams - 1; k >= 0; --k) {
    auto const& param = func.params[k];
    if (param.cppType == "HPHP::Variant" ||
        param.cppType == "HPHP::Variant const&" ||
        param.cppType == "HPHP::VRefParamValue const&") {
      continue;
    }

    if (!isFirstParam) {
      out << " &&\n" << ind << "    ";
    }
    isFirstParam = false;

    bool isOptional = (k >= func.minNumParams);
    if (isOptional) {
      out << "(count <= " << k << " || ";
    }
    if (param.cppType == "HPHP::String const&") {
      out << "IS_STRING_TYPE((args - " << k << ")->m_type)";
    } else {
      out << "(args - " << k << ")->m_type == ";
      if (param.cppType == "bool") {
        out << "KindOfBoolean";
      } else if (param.cppType == "int" || param.cppType == "long") {
        out << "KindOfInt64";
      } else if (param.cppType == "double") {
        out << "KindOfDouble";
      } else if (param.cppType == "HPHP::Array const&") {
        out << "KindOfArray";
      } else if (param.cppType == "HPHP::Object const&") {
        out << "KindOfObject";
      }
    }

    if (isOptional) {
      out << ")";
    }
  }
}


/*
 * Marshals varargs into an array.
 */
void emitBuildExtraArgs(const PhpFunc& func, std::ostream& out,
                        const char* ind) {
  out << folly::format(
    R"(
{0}Array extraArgs;
{0}{{
{0}  ArrayInit ai(count-{1});
{0}  for (int32_t i = {1}; i < count; ++i) {{
{0}    TypedValue* extraArg = ar->getExtraArg(i-{1});
{0}    if (tvIsStronglyBound(extraArg)) {{
{0}      ai.setRef(i-{1}, tvAsVariant(extraArg));
{0}    }} else {{
{0}      ai.set(i-{1}, tvAsVariant(extraArg));
{0}    }}
{0}  }}
{0}  extraArgs = ai.create();
{0}}}
)",
    ind,
    func.maxNumParams
  );
}


void emitCallExpression(const PhpFunc& func, const fbstring& prefix,
                        std::ostream& out) {
  out << prefix << func.getUniqueName() << '(';

  bool isFirstParam = true;
  if (!g_armMode && isTypeCppIndirectPass(func.returnCppType)) {
    isFirstParam = false;
    if (func.returnCppType == "HPHP::Variant") {
      out << "rv";
    } else {
      out << "&(rv->m_data)";
    }
  }

  if (!func.className.empty() && !func.isStatic) {
    if (!isFirstParam) {
      out << ", ";
    }
    isFirstParam = false;
    out << "(this_)";
  }

  if (func.isVarargs) {
    if (!isFirstParam) {
      out << ", ";
    }
    isFirstParam = false;
    out << "count";
  }

  for (auto k = 0; k < func.params.size(); ++k) {
    auto const& param = func.params[k];
    if (!isFirstParam) {
      out << ", ";
    }
    isFirstParam = false;

    if (!param.defVal.empty()) {
      out << "(count > " << k << ") ? ";
    }

    if (isTypeCppIndirectPass(param.cppType)) {
      if (param.cppType == "HPHP::Variant" ||
          param.cppType == "HPHP::Variant const&" ||
          param.cppType == "HPHP::VRefParamValue const&") {
        out << "(args-" << k << ')';
      } else {
        out << "&args[-" << k << "].m_data";
      }
    } else {
      if (param.cppType == "double") {
        out << "(args[-" << k << "].m_data.dbl)";
      } else {
        out << '(' << param.cppType << ")(args[-" << k << "].m_data.num)";
      }
    }

    if (!param.defVal.empty()) {
      out << " : ";
      if (param.defValNeedsVariable()) {
        if (param.cppType == "HPHP::Variant" ||
            param.cppType == "HPHP::Variant const&" ||
            param.cppType == "HPHP::VRefParamValue const&") {
          out << "(TypedValue*)(&defVal" << k << ')';
        } else {
          out << "(Value*)(&defVal" << k << ')';
        }
      } else if (isTypeCppIndirectPass(param.cppType)) {
        if (param.cppType == "HPHP::Variant" ||
            param.cppType == "HPHP::Variant const&" ||
            param.cppType == "HPHP::VRefParamValue const&") {
          out << "(TypedValue*)(&" << param.defVal << ')';
        } else {
          out << "(Value*)(&" << param.defVal << ')';
        }
      } else {
        out << '(' << param.cppType << ")(" << param.defVal << ')';
      }
    }
  }

  if (func.isVarargs) {
    assert(!isFirstParam);
    out << ", (Value*)(&extraArgs)";
  }
  out << ')';
}

///////////////////////////////////////////////////////////////////////////////
// Code emission helpers -- non-leaf functions

/*
 * Marshals varargs into an array if necessary, emits variables for default
 * values if necessary, and emits the call itself.
 */
void emitExtCall(const PhpFunc& func, std::ostream& out, const char* ind) {
  fbstring call_prefix;
  fbstring call_suffix;

  // Set up the type of the return value, and emit post-call code to normalize
  // return types and values
  if (func.returnCppType == "bool") {
    out << ind << "rv->m_type = KindOfBoolean;\n";
    call_prefix = "rv->m_data.num = (";
    call_suffix = ") ? 1LL : 0LL;\n";
  } else if (func.returnCppType == "int" || func.returnCppType == "long") {
    out << ind << "rv->m_type = KindOfInt64;\n";
    call_prefix = "rv->m_data.num = (int64_t)";
    call_suffix = ";\n";
  } else if (func.returnCppType == "double") {
    out << ind << "rv->m_type = KindOfDouble;\n";
    call_prefix = "rv->m_data.dbl = ";
    call_suffix = ";\n";
  } else if (func.returnCppType == "void") {
    out << ind << "rv->m_type = KindOfNull;\n";
    call_suffix = ";\n";
  } else if (func.returnCppType == "HPHP::String") {
    out << ind << "rv->m_type = KindOfString;\n";
    call_suffix = (fbstring(";\n") + ind +
                   "if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;\n");
  } else if (func.returnCppType == "HPHP::Array") {
    out << ind << "rv->m_type = KindOfArray;\n";
    call_suffix = (fbstring(";\n") + ind +
                   "if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;\n");
  } else if (func.returnCppType == "HPHP::Object") {
    out << ind << "rv->m_type = KindOfObject;\n";
    call_suffix = (fbstring(";\n") + ind +
                   "if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;\n");
  } else if (func.returnCppType == "HPHP::Variant") {
    call_suffix = (fbstring(";\n") + ind +
                   "if (rv->m_type == KindOfUninit) "
                   "rv->m_type = KindOfNull;\n");
  }

  if (func.isVarargs) {
    emitBuildExtraArgs(func, out, ind);
  }

  // If any default values need variables (because they have nontrivial values),
  // declare and initialize those
  for (auto k = 0; k < func.params.size(); ++k) {
    auto const& param = func.params[k];
    if (param.defValNeedsVariable()) {
      auto type = param.cppType;
      if (type.compare(type.length() - 7, 7, " const&") == 0) {
        type = type.substr(0, type.length() - 7);
      }
      if (type.compare(0, 6, "HPHP::") == 0) {
        type = type.substr(6);
      }
      out << ind << type << " defVal" << k;
      if (type != "Variant" ||
          (param.defVal != "null" && param.defVal != "null_variant")) {
        out << " = ";
        out << (param.defVal == "null" ? "uninit_null()" : param.defVal);
      }
      out << ";\n";
    }
  }

  // Put the return-value-space pointer into x8
  if (g_armMode) {
    out << ind << "asm volatile (\"mov x8, %0\\n\" : : \"r\"(";
    if (func.returnCppType == "HPHP::Variant") {
      out << "rv";
    } else {
      out << "&(rv->m_data)";
    }
    out << ") : \"x8\");\n";
  }

  out << ind << call_prefix;
  emitCallExpression(func, func.className.empty() ? "fh_" : "th_", out);
  out << call_suffix;
}

void emitCasts(const PhpFunc& func, std::ostream& out, const char* ind) {
  assert(func.numTypeChecks > 0);

  if (func.numTypeChecks == 1) {
    for (auto i = func.maxNumParams - 1; i >= 0; --i) {
      if (func.params[i].isCheckedType()) {
        emitCast(func.params[i], i, out, ind, false);
        return;
      }
    }
    assert(false); // not reached
  }

  if (func.minNumParams != func.maxNumParams) {
    out << ind << "switch (count) {\n";
    for (auto i = func.maxNumParams - 1; i >= func.minNumParams; --i) {
      auto const& param = func.params[i];
      if (i == func.maxNumParams - 1) {
        out << ind << "default: // count >= " << func.maxNumParams << '\n';
      } else {
        out << ind << "case " << (i + 1) << ":\n";
      }
      if (param.isCheckedType()) {
        emitCast(param, i, out, ind - 2, true);
      }
    }
    out << ind << "case " << func.minNumParams << ":\n";
    out << ind << "  break;\n";
    out << ind << "}\n";
  }
  for (auto i = func.minNumParams - 1; i >= 0; --i) {
    auto const& param = func.params[i];
    if (param.isCheckedType()) {
      emitCast(param, i, out, ind, true);
    }
  }
}


/*
 * Emits the fg1_ helper, which assumes that the arg count is acceptable, but at
 * least one typecheck has failed. It will cast arguments to the appropriate
 * types and then call the fh_ alias.
 */
void emitSlowPathHelper(const PhpFunc& func, const fbstring& prefix,
                        std::ostream& out) {
  out << "void " << prefix << func.getUniqueName()
      << "(TypedValue* rv, ActRec* ar, int32_t count";
  if (func.isMethod() && !func.isStatic) {
    out << ", ObjectData* this_";
  }
  out << ") __attribute__((noinline,cold));\n";

  out << "void " << prefix << func.getUniqueName()
      << "(TypedValue* rv, ActRec* ar, int32_t count";
  if (func.isMethod() && !func.isStatic) {
    out << ", ObjectData* this_";
  }
  out << ") {\n";

  const char* eightSpaces = "        ";
  const char* ind = eightSpaces + 6;

  out << ind << "TypedValue* args UNUSED = ((TypedValue*)ar) - 1;\n";

  emitCasts(func, out, ind);
  emitExtCall(func, out, ind);

  out << "}\n\n";
}


///////////////////////////////////////////////////////////////////////////////
// Top level

/*
 * Called for each line on stdin. Looks up the symbol's characteristics in the
 * IDL, and emits up to three things:
 *
 * - An fh_* declaration, which is the f_* signature with ABI exposed.
 * - [Maybe] an fg1_* stub which does type casting of arguments, then calls fh_.
 * - An fg_ stub which checks arg counts and types, then calls fh_ or fg1_.
 */
void processSymbol(const fbstring& symbol, std::ostream& header,
                   std::ostream& cpp) {
  int status;
  auto demangled = abi::__cxa_demangle(symbol.c_str(), nullptr, 0, &status);
  SCOPE_EXIT { free(demangled); };

  if (status != 0) {
    return;
  }

  auto idlIt = g_mangleMap.find(demangled);
  if (idlIt == g_mangleMap.end()) {
    // A symbol that doesn't correspond to anything in the IDL.
    return;
  }

  auto& func = *idlIt->second;
  bool isMethod = !func.className.empty();

  auto classIt = g_classMap.find(func.className);
  if (isMethod && func.name == "__construct" &&
      classIt != g_classMap.end()) {
    auto& klass = *classIt->second;
    auto& flags = klass.flags;

    if (std::find(flags.begin(), flags.end(), "IsCppAbstract") == flags.end()) {
      emitCtorHelper(klass.name, cpp);
    }
    if (std::find(flags.begin(), flags.end(), "NoDefaultSweep")
        != flags.end()) {
      cpp << "IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(" << klass.name << ");\n";
    } else {
      cpp << "IMPLEMENT_CLASS(" << klass.name << ");\n";
    }
  }

  fbstring declPrefix = (isMethod ? "th_" : "fh_");
  fbstring slowPathPrefix = (isMethod ? "tg1_" : "fg1_");
  fbstring stubPrefix = (isMethod ? "tg_" : "fg_");

  std::ostringstream decl;
  emitRemappedFuncDecl(func, symbol, declPrefix, decl);
  if (!isMethod) {
    header << decl.str();
  }
  cpp << decl.str();

  if (func.numTypeChecks > 0) {
    emitSlowPathHelper(func, slowPathPrefix, cpp);
  }

  // This is how we change the indentation level. So clever.
  const char* eightSpaces = "        ";
  const char* in = eightSpaces + 6;

  cpp << "TypedValue* " << stubPrefix << func.getUniqueName()
      << "(ActRec* ar) {\n";
  cpp << in << "TypedValue rvSpace;\n";
  cpp << in << "TypedValue* rv = &rvSpace;\n";
  cpp << in << "int32_t count = ar->numArgs();\n";
  cpp << in << "TypedValue* args UNUSED = ((TypedValue*)ar) - 1;\n";

  if (func.isMethod() && !func.isStatic) {
    cpp << in
        << "ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);\n";
    cpp << in << "if (this_) {\n";
    in -= 2;
  }

  // Check the arg count
  bool needArgMiscountClause = false;
  if (func.isVarargs) {
    if (func.minNumParams > 0) {
      cpp << in << "if (count >= " << func.minNumParams << ") {\n";
      needArgMiscountClause = true;
      in -= 2;
    }
  } else {
    if (func.minNumParams == func.maxNumParams) {
      cpp << in << "if (count == " << func.minNumParams << ") {\n";
    } else if (func.minNumParams == 0) {
      cpp << in << "if (count <= " << func.maxNumParams << ") {\n";
    } else {
      cpp << in << "if (count >= " << func.minNumParams
          << " && count <= "<< func.maxNumParams << ") {\n";
    }
    needArgMiscountClause = true;
    in -= 2;
  }

  // Count is OK. Check arg types
  if (func.numTypeChecks > 0) {
    cpp << in << "if (";
    emitTypechecks(func, cpp, in);
    cpp << ") {\n";
    in -= 2;
  }

  // Call the f_ function via the fh_ alias
  emitExtCall(func, cpp, in);

  // Deal with type mismatches: punt to fg1_
  if (func.numTypeChecks > 0) {
    cpp << in + 2 << "} else {\n";
    cpp << in << slowPathPrefix << func.getUniqueName() << "(rv, ar, count";
    if (func.isMethod() && !func.isStatic) {
      cpp << ", this_";
    }
    cpp << ");\n";
    in += 2;
    cpp << in << "}\n";
  }

  if (needArgMiscountClause) {
    cpp << in + 2 << "} else {\n";
    if (func.isVarargs) {
      cpp << in << "throw_missing_arguments_nr(\"" << func.getPrettyName()
          << "\", " << func.minNumParams << ", count, 1);\n";
    } else {
      if (func.minNumParams == 0) {
        cpp << in << "throw_toomany_arguments_nr(\"" << func.getPrettyName()
            << "\", " << func.maxNumParams << ", 1);\n";
      } else {
        cpp << in << "throw_wrong_arguments_nr(\"" << func.getPrettyName()
            << "\", count, " << func.minNumParams << ", "
            << func.maxNumParams << ", 1);\n";
      }
    }
    cpp << in << "rv->m_data.num = 0LL;\n";
    cpp << in << "rv->m_type = KindOfNull;\n";
    in += 2;
    cpp << in << "}\n";
  }

  if (func.isMethod() && !func.isStatic) {
    cpp << in + 2 << "} else {\n";
    cpp << in << "throw_instance_method_fatal(\"" << func.className
        << "::" << func.name << "\");\n";
    in += 2;
    cpp << in << "}\n";
  }

  auto numLocals = func.maxNumParams;
  bool noThis = (func.className.empty() || func.isStatic);
  auto frameFree =
    (noThis ? "frame_free_locals_no_this_inl" : "frame_free_locals_inl");
  cpp << in << frameFree << "(ar, " << numLocals << ");\n";
  cpp << in << "memcpy(&ar->m_r, rv, sizeof(TypedValue));\n";
  cpp << in << "return &ar->m_r;\n";
  cpp << "}\n\n";
}

int main(int argc, const char* argv[]) {
  if (argc < 5) {
    std::cout << "Usage: " << argv[0]
              << " <x64|arm> <output .h> <output .cpp> <*.idl.json>...\n"
              << "Pipe mangled C++ symbols to stdin.\n";
    return 0;
  }

  g_armMode = (strcmp(argv[1], "arm") == 0);

  std::ofstream header(argv[2]);
  std::ofstream cpp(argv[3]);

  fbvector<PhpFunc> funcs;
  fbvector<PhpClass> classes;

  for (auto i = 4; i < argc; ++i) {
    try {
      parseIDL(argv[i], funcs, classes);
    } catch (const std::exception& exc) {
      std::cerr << argv[i] << ": " << exc.what() << "\n";
      return 1;
    }
  }

  for (auto const& func : funcs) {
    g_mangleMap[func.getCppSig()] = &func;
  }
  for (auto const& klass : classes) {
    g_classMap[klass.name] = &klass;
    for (auto const& func : klass.methods) {
      g_mangleMap[func.getCppSig()] = &func;
    }
  }

  header << "namespace HPHP {\n\n";
  cpp << g_allIncludes << "\n";
  cpp << "namespace HPHP {\n\n";

  std::string line;
  while (std::getline(std::cin, line)) {
    processSymbol(line, header, cpp);
  }

  header << "} // namespace HPHP\n";
  cpp << "} // namespace HPHP\n";

  return 0;
}
