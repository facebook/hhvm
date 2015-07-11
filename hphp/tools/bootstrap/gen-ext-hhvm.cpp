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
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/algorithm/string/predicate.hpp>

#include <folly/FBString.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>

#include "hphp/tools/bootstrap/idl.h"

using folly::fbstring;
using namespace HPHP::IDL;
using namespace HPHP;

std::vector<const PhpFunc*> g_funcVec;
std::unordered_map<fbstring, const PhpClass*> g_classMap;

// Functions with return types that don't fit in registers are handled
// differently on ARM. Instead of using a pointer-to-return-value-space hidden
// first parameter like on x64, the pointer is passed in a register reserved for
// this purpose, not part of the normal argument sequence.
bool g_armMode = false;

const char* const g_allIncludes = R"(
#include "hphp/runtime/ext_hhvm/ext_hhvm.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/ext.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/util/abi-cxx.h"
#include <exception>
)";

///////////////////////////////////////////////////////////////////////////////
// Code emission helpers -- leaf functions

void emitCtorHelper(const fbstring& className, std::ostream& out) {
  out << folly::format(
    R"(
ObjectData* new_{0:s}_Instance(HPHP::Class* cls) {{
  size_t nProps = cls->numDeclProperties();
  size_t builtinObjSize = sizeof(c_{0:s}) - sizeof(ObjectData);
  size_t size = ObjectData::sizeForNProps(nProps) + builtinObjSize;
  auto o = new (MM().objMalloc(size)) c_{0:s}(cls);
  assert(o->hasExactlyOneRef());
  return o;
}})",
    className) << "\n\n";
}

void emitDtorHelper(const fbstring& className, std::ostream& out) {
  out << folly::format(
    R"(
void delete_{0:s}(ObjectData* obj, const Class* cls) {{
  auto const ptr = static_cast<c_{0:s}*>(obj);
  ptr->~c_{0:s}();

  auto const nProps = cls->numDeclProperties();
  auto const propVec = reinterpret_cast<TypedValue*>(ptr + 1);
  for (auto i = Slot{{0}}; i < nProps; ++i) {{
    tvRefcountedDecRef(&propVec[i]);
  }}

  auto const builtinSz = sizeof(c_{0:s}) - sizeof(ObjectData);
  auto const size = ObjectData::sizeForNProps(nProps) + builtinSz;
  if (LIKELY(size <= kMaxSmallSize)) {{
    return MM().freeSmallSize(ptr, size);
  }}
  return MM().freeBigSize(ptr, size);
}})",
    className) << "\n\n";
}

/*
 * Emits a function pointer type that corresponds to a given f_* function or
 * t_* or ti_* method, but with types and signatures adjusted to reflect the
 * underlying C++ ABI.
 */
void emitFuncPtrType(const PhpFunc& func,
                     const fbstring& name,
                     std::ostream& out) {
  int returnKindOf = func.returnKindOf();
  bool indirectReturn = func.isIndirectReturn();
  if (indirectReturn) {
    if (returnKindOf == KindOfAny) {
      out << "TypedValue* ";
    } else {
      out << "Value* ";
    }
  } else {
    out << func.returnCppType() << ' ';
  }

  out << "(*" << name << ")(";

  bool isFirstParam = true;

  if (!g_armMode && indirectReturn) {
    if (func.returnKindOf() == KindOfAny) {
      out << "TypedValue*";
    } else {
      out << "Value*";
    }
    isFirstParam = false;
  }

  if (func.usesThis()) {
    if (!isFirstParam) {
      out << ",";
    }
    out << "c_" << func.className() << "*";
    isFirstParam = false;
  }

  if (func.isVarArgs()) {
    if (!isFirstParam) {
      out << ",";
    }
    out << "int64_t";
    isFirstParam = false;
  }

  for (auto const& param : func.params()) {
    if (!isFirstParam) {
      out << ",";
    }
    auto kindof = param.kindOf();
    if (param.isIndirectPass()) {
      if (kindof == KindOfAny || kindof == KindOfRef) {
        out << "TypedValue*";
      } else {
        out << "Value*";
      }
    } else {
      out << param.getCppType();
    }
    isFirstParam = false;
  }

  if (func.isVarArgs()) {
    assert(!isFirstParam);
    out << ",Value*";
  }

  out << ")";
}

static void emitFuncPtrDecl(const PhpFunc& func,
                            const fbstring& ind,
                            std::ostream& out) {
  out << ind;
  fbstring declPrefix = (func.isMethod() ? "th_" : "fh_");
  emitFuncPtrType(func, declPrefix + func.getUniqueName(), out);
  out << " =\n";
  out << ind << "  (";
  emitFuncPtrType(func, "", out);
  out << ")" << (func.isMethod() ? "getMethodPtr" : "")
      << "(&" << func.getPrefixedCppName() << ");\n";
}

static void emitZendParamPrefix(std::ostream& out,
                                int32_t index,
                                const PhpParam& param,
                                const char* ind) {
  out << ind << "if (!"
        << "tvCoerceParamTo" << kindOfString(param.kindOf())
        << "InPlace(args-" << index << ")) {\n"
      << ind << "  raise_param_type_warning(__func__, " << index << " + 1, "
        << "KindOf" << kindOfString(param.kindOf()) << ", "
        << "(args-" << index << ")->m_type);\n";
}

static void emitZendParamSuffix(std::ostream& out, const char* ind) {
  out << ind << "  return;\n"
      << ind << "}\n";
}

void emitCast(const PhpParam& param, int32_t index, std::ostream& out,
              const char* ind, bool doCheck) {
  if (doCheck) {
    out << ind << "if (";
    if (param.kindOf() == KindOfString) {
      out << "!IS_STRING_TYPE((args-" << index << ")->m_type)";
    } else {
      out << "(args-" << index << ")->m_type != KindOf"
          << kindOfString(param.kindOf());
    }
    out << ") {\n";
    ind -= 2;
  }


  if (param.getParamMode() == ParamMode::ZendNull) {
    emitZendParamPrefix(out, index, param, ind);
    out << ind << "  rv->m_type = KindOfUninit;\n";
    emitZendParamSuffix(out, ind);
  } else if (param.getParamMode() == ParamMode::ZendFalse) {
    emitZendParamPrefix(out, index, param, ind);
    out << ind << "  rv->m_type = KindOfBoolean;\n"
        << ind << "  rv->m_data.num = 0;\n";
    emitZendParamSuffix(out, ind);
  } else if (param.kindOf() == KindOfObject && param.hasDefault()) {
    out << ind << "tvCastToNullableObject"
        << "InPlace(args-" << index << ");\n";
  } else if (param.kindOf() != KindOfAny) {
    out << ind << "tvCastTo" << kindOfString(param.kindOf())
        << "InPlace(args-" << index << ");\n";
  }

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
  for (int k = func.numParams() - 1; k >= 0; --k) {
    auto const& param = func.param(k);
    auto kindof = param.kindOf();
    if (kindof == KindOfAny || kindof == KindOfRef) {
      continue;
    }

    if (!isFirstParam) {
      out << " &&\n" << ind << "    ";
    }
    isFirstParam = false;

    bool isOptional = (k >= func.minNumParams());
    if (isOptional) {
      out << "(count <= " << k << " || ";
    }
    if (kindof == KindOfString) {
      out << "IS_STRING_TYPE((args - " << k << ")->m_type)";
    } else {
      out << "(args - " << k << ")->m_type == KindOf" << kindOfString(kindof);
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
{0}if (count > {1}) {{
{0}  ArrayInit ai((size_t)count-{1}, ArrayInit::Mixed{{}});
{0}  for (int32_t i = {1}; i < count; ++i) {{
{0}    TypedValue* extraArg = ar->getExtraArg(i-{1});
{0}    if (tvIsStronglyBound(extraArg)) {{
{0}      ai.setRef(i-{1}, tvAsVariant(extraArg));
{0}    }} else {{
{0}      ai.set(i-{1}, tvAsVariant(extraArg));
{0}    }}
{0}  }}
{0}  extraArgs = ai.toArray();
{0}}}
)",
    ind,
    func.numParams()
  );
}


void emitCallExpression(const PhpFunc& func, const fbstring& prefix,
                        std::ostream& out) {
  out << prefix << func.getUniqueName() << '(';

  bool isFirstParam = true;
  if (!g_armMode && func.isIndirectReturn()) {
    isFirstParam = false;
    if (func.returnKindOf() == KindOfAny) {
      out << "rv";
    } else {
      out << "&(rv->m_data)";
    }
  }

  if (func.usesThis()) {
    if (!isFirstParam) {
      out << ", ";
    }
    isFirstParam = false;
    out << "(this_)";
  }

  if (func.isVarArgs()) {
    if (!isFirstParam) {
      out << ", ";
    }
    isFirstParam = false;
    out << "count";
  }

  for (auto k = 0; k < func.numParams(); ++k) {
    auto const& param = func.param(k);
    if (!isFirstParam) {
      out << ", ";
    }
    isFirstParam = false;

    if (param.hasDefault()) {
      out << "(count > " << k << ") ? ";
    }

    if (param.isIndirectPass()) {
      auto kindof = param.kindOf();
      if (kindof == KindOfAny || kindof == KindOfRef) {
        out << "(args-" << k << ')';
      } else {
        out << "&args[-" << k << "].m_data";
      }
    } else {
      if (param.kindOf() == KindOfDouble) {
        out << "(args[-" << k << "].m_data.dbl)";
      } else {
        out << '(' << param.getCppType() << ")(args[-" << k << "].m_data.num)";
      }
    }

    if (param.hasDefault()) {
      out << " : ";
      auto kindof = param.kindOf();
      if (param.defValueNeedsVariable()) {
        if (kindof == KindOfAny || kindof == KindOfRef) {
          out << "(TypedValue*)(&defVal" << k << ')';
        } else {
          out << "(Value*)(&defVal" << k << ')';
        }
      } else if (param.isIndirectPass()) {
        if (kindof == KindOfAny || kindof == KindOfRef) {
          out << "(TypedValue*)(&" << param.getDefault() << ')';
        } else {
          out << "(Value*)(&" << param.getDefault() << ')';
        }
      } else {
        out << '(' << param.getCppType() << ")(" << param.getDefault() << ')';
      }
    }
  }

  if (func.isVarArgs()) {
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
  auto returnKindOf = func.returnKindOf();
  if (returnKindOf == KindOfBoolean) {
    out << ind << "rv->m_type = KindOfBoolean;\n";
    call_prefix = "rv->m_data.num = (";
    call_suffix = ") ? 1LL : 0LL;\n";
  } else if (returnKindOf == KindOfInt64) {
    out << ind << "rv->m_type = KindOfInt64;\n";
    call_prefix = "rv->m_data.num = (int64_t)";
    call_suffix = ";\n";
  } else if (returnKindOf == KindOfDouble) {
    out << ind << "rv->m_type = KindOfDouble;\n";
    call_prefix = "rv->m_data.dbl = ";
    call_suffix = ";\n";
  } else if (returnKindOf == KindOfInvalid || returnKindOf == KindOfNull) {
    out << ind << "rv->m_type = KindOfNull;\n";
    call_suffix = ";\n";
  } else if (returnKindOf == KindOfString) {
    out << ind << "rv->m_type = KindOfString;\n";
    call_suffix = (fbstring(";\n") + ind +
                   "if (UNLIKELY(rv->m_data.num == 0LL)) "
                   "rv->m_type = KindOfNull;\n");
  } else if (returnKindOf == KindOfArray) {
    out << ind << "rv->m_type = KindOfArray;\n";
    call_suffix = (fbstring(";\n") + ind +
                   "if (UNLIKELY(rv->m_data.num == 0LL)) "
                   "rv->m_type = KindOfNull;\n");
  } else if (returnKindOf == KindOfObject) {
    out << ind << "rv->m_type = KindOfObject;\n";
    call_suffix = (fbstring(";\n") + ind +
                   "if (UNLIKELY(rv->m_data.num == 0LL)) "
                   "rv->m_type = KindOfNull;\n");
  } else if (returnKindOf == KindOfResource) {
    out << ind << "rv->m_type = KindOfResource;\n";
    call_suffix = (fbstring(";\n") + ind +
                   "if (UNLIKELY(rv->m_data.num == 0LL)) "
                   "rv->m_type = KindOfNull;\n");
  } else {
    call_suffix = (fbstring(";\n") + ind +
                   "if (UNLIKELY(rv->m_type == KindOfUninit)) "
                   "rv->m_type = KindOfNull;\n");
  }

  if (func.isVarArgs()) {
    emitBuildExtraArgs(func, out, ind);
  }

  // If any default values need variables (because they have nontrivial values),
  // declare and initialize those
  for (auto k = 0; k < func.numParams(); ++k) {
    auto const& param = func.param(k);
    if (param.defValueNeedsVariable()) {
      DataType kindof = param.kindOf();
      out << ind << param.getStrippedCppType() << " defVal" << k;
      fbstring defVal = param.getDefault();
      if (kindof != KindOfAny ||
          (defVal != "null" && defVal != "null_variant")) {
        out << " = ";

        if (boost::starts_with(defVal, "null")) {
          switch (kindof) {
            case KindOfArray:
              out << "Array()";
              break;
            case KindOfString:
              out << "String()";
              break;
            case KindOfResource:
              out << "Resource()";
              break;
            case KindOfObject:
              out << "Object()";
              break;
            case KindOfRef:
              out << "init_null()";
              break;
            default:
              out << "No valid null object.";
          }
        } else {
          out << defVal;
        }
      }
      out << ";\n";
    }
  }

  // Put the return-value-space pointer into x8
  if (g_armMode) {
    out << ind << "asm volatile (\"mov x8, %0\\n\" : : \"r\"(";
    if (func.returnKindOf() == KindOfAny) {
      out << "rv";
    } else {
      out << "&(rv->m_data)";
    }
    out << ") : \"x8\");\n";
  }

  out << ind << call_prefix;
  emitCallExpression(func, func.isMethod() ? "th_" : "fh_", out);
  out << call_suffix;
}

void emitCasts(const PhpFunc& func, std::ostream& out, const char* ind) {
  assert(func.numTypeChecks() > 0);

  if (func.numTypeChecks() == 1) {
    for (auto i = func.numParams() - 1; i >= 0; --i) {
      if (func.param(i).isCheckedType()) {
        emitCast(func.param(i), i, out, ind, false);
        return;
      }
    }
    assert(false); // not reached
  }

  if (func.minNumParams() != func.numParams()) {
    out << ind << "switch (count) {\n";
    for (auto i = func.numParams() - 1; i >= func.minNumParams(); --i) {
      auto const& param = func.param(i);
      if (i == func.numParams() - 1) {
        out << ind << "default: // count >= " << func.numParams() << '\n';
      } else {
        out << ind << "case " << (i + 1) << ":\n";
      }
      if (param.isCheckedType()) {
        emitCast(param, i, out, ind - 2, true);
      }
    }
    out << ind << "case " << func.minNumParams() << ":\n";
    out << ind << "  break;\n";
    out << ind << "}\n";
  }
  for (auto i = func.minNumParams() - 1; i >= 0; --i) {
    auto const& param = func.param(i);
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
  if (func.usesThis()) {
    out << ", c_" << func.className() << "* this_";
  }
  out << ") __attribute__((__noinline__,cold));\n";

  out << "void " << prefix << func.getUniqueName()
      << "(TypedValue* rv, ActRec* ar, int32_t count";
  if (func.usesThis()) {
    out << ", c_" << func.className() << "* this_";
  }
  out << ") {\n";

  const char* eightSpaces = "        ";
  const char* ind = eightSpaces + 6;

  emitFuncPtrDecl(func, ind, out);

  out << ind << "TypedValue* args UNUSED = ((TypedValue*)ar) - 1;\n";

  emitCasts(func, out, ind);
  emitExtCall(func, out, ind);

  out << "}\n\n";
}

/**
 * Emits all the methods that are needed for class creation
 * */
static void emitClassCtorAndDtor(const PhpClass& klass, std::ostream& out) {
  if (!(klass.flags() & IsCppAbstract)) {
    emitCtorHelper(klass.getCppName(), out);
    if (!(klass.flags() & CppCustomDelete)) {
      emitDtorHelper(klass.getCppName(), out);
    }
  }
  if (klass.flags() & NoDefaultSweep) {
    out << "IMPLEMENT_CLASS_NO_SWEEP(" << klass.getCppName() << ");\n";
  } else {
    out << "IMPLEMENT_CLASS(" << klass.getCppName() << ");\n";
  }
}

///////////////////////////////////////////////////////////////////////////////
// Top level

/*
 * Looks up a function's characteristics in the IDL, and emits up to three
 * things:
 *
 * - An fg_ or tg_ stub which checks arg counts and types, then calls
 *   fg1_/tg1_ or the fh_/th_ alias.
 * - [Maybe] an fg1_ or tg1_ stub which does type casting of arguments, then
 *   calls fh_/th_.
 * - [Maybe] an extern declaration for the f_ function if it's not a method
 *
 * Inside the bodies of the fg_/tg_ stub and the fg1_/tg1_ stub, we emit a
 * fh_ or th_ function pointer alias, which is the f_/t_ signature with ABI
 * exposed.
 */
void processFunc(const PhpFunc& func, std::ostream& header,
                 std::ostream& cpp) {
  bool isMethod = func.isMethod();
  auto classIt = g_classMap.find(func.className());
  if (classIt != g_classMap.end()) {
    auto& klass = *classIt->second;
    emitClassCtorAndDtor(klass, cpp);
    g_classMap.erase(classIt);
  }

  fbstring slowPathPrefix = (isMethod ? "tg1_" : "fg1_");
  fbstring stubPrefix = (isMethod ? "tg_" : "fg_");

  if (!isMethod) {
    cpp << "extern " << func.returnCppType() << " "
        << func.getCppSig(false) << ";\n\n";
  }

  if (func.numTypeChecks() > 0) {
    emitSlowPathHelper(func, slowPathPrefix, cpp);
  }

  // This is how we change the indentation level. So clever.
  const char* eightSpaces = "        ";
  const char* in = eightSpaces + 6;

  cpp << "TypedValue* " << stubPrefix << func.getUniqueName()
      << "(ActRec* ar) {\n";

  emitFuncPtrDecl(func, in, cpp);

  cpp << in << "TypedValue rvSpace;\n";
  cpp << in << "TypedValue* rv = &rvSpace;\n";
  cpp << in << "int32_t count = ar->numArgs();\n";
  cpp << in << "TypedValue* args UNUSED = ((TypedValue*)ar) - 1;\n";

  if (func.usesThis()) {
    auto cklass = "c_" + func.className();
    cpp << in
        << cklass << "* this_ = (ar->hasThis() ? "
        << "static_cast<" << cklass << "*>(ar->getThis()) : "
        << " nullptr);\n";
    cpp << in << "if (LIKELY(this_ != nullptr)) {\n";
    in -= 2;
  }

  // Check the arg count
  bool needArgMiscountClause = false;
  if (func.isVarArgs()) {
    if (func.minNumParams() > 0) {
      cpp << in << "if (LIKELY(count >= " << func.minNumParams() << ")) {\n";
      needArgMiscountClause = true;
      in -= 2;
    }
  } else {
    if (func.minNumParams() == func.numParams()) {
      cpp << in << "if (LIKELY(count == " << func.minNumParams() << ")) {\n";
    } else if (func.minNumParams() == 0) {
      cpp << in << "if (LIKELY(count <= " << func.numParams() << ")) {\n";
    } else {
      cpp << in << "if (LIKELY(count >= " << func.minNumParams()
          << " && count <= "<< func.numParams() << ")) {\n";
    }
    needArgMiscountClause = true;
    in -= 2;
  }

  // Count is OK. Check arg types
  if (func.numTypeChecks() > 0) {
    cpp << in << "if (LIKELY(";
    emitTypechecks(func, cpp, in);
    cpp << ")) {\n";
    in -= 2;
  }

  // Call the f_ function via the fh_ alias
  emitExtCall(func, cpp, in);
  if (needArgMiscountClause && (func.numParams() == 0) && func.usesThis()) {
    cpp << in << "frame_free_inl(ar, rv);\n";
    cpp << in << "ar->m_r = *rv;\n";
    cpp << in << "return &ar->m_r;\n";
  }

  // Deal with type mismatches: punt to fg1_
  if (func.numTypeChecks() > 0) {
    cpp << in + 2 << "} else {\n";
    cpp << in << slowPathPrefix << func.getUniqueName() << "(rv, ar, count";
    if (func.usesThis()) {
      cpp << ", this_";
    }
    cpp << ");\n";
    in += 2;
    cpp << in << "}\n";
  }

  if (needArgMiscountClause) {
    cpp << in + 2 << "} else {\n";
    if (func.isVarArgs()) {
      cpp << in << "throw_missing_arguments_nr(\""
          << escapeCpp(func.getPrettyName())
          << "\", " << func.minNumParams() << ", count, 1, rv);\n";
    } else {
      if (func.minNumParams() == 0) {
        cpp << in << "throw_toomany_arguments_nr(\""
            << escapeCpp(func.getPrettyName())
            << "\", " << func.numParams() << ", count, 1, rv);\n";
      } else {
        cpp << in << "throw_wrong_arguments_nr(\""
            << escapeCpp(func.getPrettyName())
            << "\", count, " << func.minNumParams() << ", "
            << func.numParams() << ", 1, rv);\n";
      }
    }
    in += 2;
    cpp << in << "}\n";
  }

  if (func.isMethod() && !func.isStatic()) {
    cpp << in + 2 << "} else {\n";
    cpp << in << "throw_instance_method_fatal(\"" << func.className()
        << "::" << func.getCppName() << "\");\n";
    in += 2;
    cpp << in << "}\n";
  }

  auto numLocals = func.numParams();
  auto frameFree =
    func.usesThis() ? "frame_free_locals_inl" : "frame_free_locals_no_this_inl";
  cpp << in << frameFree << "(ar, " << numLocals << ", rv);\n";
  cpp << in << "ar->m_r = *rv;\n";
  cpp << in << "return &ar->m_r;\n";
  cpp << "}\n\n";
}

int main(int argc, const char* argv[]) {
  if (argc < 5) {
    std::cout << "Usage: " << argv[0]
              << " <x64|arm> <output .h> <output .cpp> <*.idl.json>...\n";
    return 0;
  }

  fbstring invocation_trace;
  makeInvocationTrace(invocation_trace, argc, argv);

  g_armMode = (strcmp(argv[1], "arm") == 0);

  std::ofstream header(argv[2]);
  std::ofstream cpp(argv[3]);

  brandOutputFile(header, "gen-ext-hhvm.cpp", invocation_trace);
  brandOutputFile(cpp, "gen-ext-hhvm.cpp", invocation_trace);

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
    g_funcVec.push_back(&func);
  }
  for (auto const& klass : classes) {
    g_classMap[klass.getCppName()] = &klass;
    for (auto const& func : klass.methods()) {
      g_funcVec.push_back(&func);
    }
  }

  header << "namespace HPHP {\n\n";
  cpp << g_allIncludes << "\n";
  cpp << "namespace HPHP {\n\n";

  auto idlIt = g_funcVec.begin();
  for (; idlIt != g_funcVec.end(); ++idlIt) {
    processFunc(**idlIt, header, cpp);
  }

  header << "} // namespace HPHP\n";
  cpp << "} // namespace HPHP\n";

  return 0;
}
