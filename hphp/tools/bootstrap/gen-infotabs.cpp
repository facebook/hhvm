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

void write_zend_func_stub(std::ofstream& cpp, PhpFunc func,
                          fbstring class_name = "") {
  const auto& real_prefix = class_name.empty() ? "zif_" : "zim_";
  const auto& stub_prefix = class_name.empty() ? "fg_" : "tg_";
  if (!class_name.empty()) {
    class_name = class_name + "_";
  }
  cpp << folly::format(R"(
}} // End namespace
void {0}{1}{2}(
    HPHP::ActRec* ar,
    HPHP::RefData* return_value,
    HPHP::RefData* this_ptr);
namespace HPHP {{
TypedValue* {3}{1}{2}(ActRec* ar) {{
  return zend_wrap_func(ar, {0}{1}{2}, {4}, {5});
}}
)", real_prefix, class_name, func.name(), stub_prefix,
    func.numParams(), func.isReturnRef());
}

int main(int argc, const char* argv[]) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <output file> <*.idl.json>...\n";
    return 0;
  }

  fbvector<PhpFunc> funcs;
  fbvector<PhpClass> classes;

  for (auto i = 2; i < argc; ++i) {
    try {
      parseIDL(argv[i], funcs, classes);
    } catch (const std::exception& exc) {
      std::cerr << argv[i] << ": " << exc.what() << "\n";
      return 1;
    }
  }

  std::ofstream cpp(argv[1]);

  cpp << "#include \"hphp/runtime/ext_hhvm/ext_hhvm.h\"\n"
      << "#include \"hphp/runtime/ext_hhvm/ext_zend_compat.h\"\n"
      << "#include \"hphp/runtime/ext/ext.h\"\n"
      << "#include \"hphp/runtime/vm/runtime.h\"\n"
      << "#include \"ext_hhvm_infotabs.h\"\n"
      << "namespace HPHP {\n"
      << "  struct TypedValue;\n"
      << "  struct ActRec;\n"
      << "  class Class;\n"
      << "\n\n";

  std::unordered_set<fbstring> classesWithCtors;

  ////////////////////////////////
  // Declare the fg_ and tg_ stubs

  for (auto const& func : funcs) {
    fbstring name = func.lowerName();
    if (func.flags() & ZendCompat) {
      write_zend_func_stub(cpp, func);
    } else {
      cpp << "TypedValue* fg_" << name << "(ActRec* ar);\n";
    }
  }

  for (auto const& klass : classes) {
    if (!(klass.flags() & IsCppAbstract) && !(klass.flags() & ZendCompat)) {
      cpp << "ObjectData* new_" << klass.name() << "_Instance(Class*);\n";
      classesWithCtors.insert(klass.name());
    }
    for (auto const& func : klass.methods()) {
      if (func.flags() & ZendCompat) {
        write_zend_func_stub(cpp, func, klass.lowerName());
      } else {
        cpp << "TypedValue* tg_" << func.getUniqueName()
            << "(ActRec* ar);\n";
      }
    }
  }

  cpp << "\n";
  cpp << "}\n";
  cpp << "namespace HPHP {\n";

  ///////////////////////////////////////
  // Define the name - fg_ - fh_ mappings

  cpp << "const long long hhbc_ext_funcs_count = " << funcs.size() << ";\n";
  cpp << "const HhbcExtFuncInfo hhbc_ext_funcs[] = {\n  ";

  bool first = true;
  for (auto const& func : funcs) {
    if (func.isMethod()) {
      continue;
    }
    if (!first) {
      cpp << ",\n  ";
    }
    first = false;

    auto prefix = "fh_";
    if (func.flags() & ZendCompat) {
      prefix = "zif_";
    }

    fbstring name = func.lowerName();
    cpp << "{ \"" << name << "\", " << "fg_" << name
        << ", (void *)&" << prefix << name << " }";
  }
  cpp << "\n};\n\n";

  for (auto const& klass : classes) {
    cpp << "static const long long hhbc_ext_method_count_" << klass.name()
        << " = " << klass.numMethods() << ";\n";
    cpp << "static const HhbcExtMethodInfo hhbc_ext_methods_" << klass.name()
        << "[] = {\n  ";
    first = true;
    for (auto const& method : klass.methods()) {
      if (!first) {
        cpp << ",\n  ";
      }
      first = false;

      auto name = method.getUniqueName();
      if (method.flags() & ZendCompat) {
        name = klass.lowerName() + "_" + method.name();
      }
      cpp << "{ \"" << method.name() << "\", tg_" << name << " }";
    }
    cpp << "\n};\n\n";
  }

  cpp << "const long long hhbc_ext_class_count = " << classes.size() << ";\n";

  cpp << "const HhbcExtClassInfo hhbc_ext_classes[] = {\n  ";
  first = true;
  for (auto const& klass : classes) {
    if (!first) {
      cpp << ",\n  ";
    }
    first = false;

    auto ctor = (classesWithCtors.count(klass.name()) > 0
                 ? fbstring("new_") + klass.name() + "_Instance"
                 : fbstring("nullptr"));

    auto cpp_name = klass.name();
    if (klass.flags() & ZendCompat) {
      cpp_name = "ZendObjectData";
      ctor = "new_ZendObjectData_Instance";
    }

    cpp << "{ \"" << klass.name() << "\", " << ctor
        << ", sizeof(c_" << cpp_name << ')'
        << ", hhbc_ext_method_count_" << klass.name()
        << ", hhbc_ext_methods_" << klass.name()
        << ", &c_" << cpp_name << "::classof() }";
  }
  cpp << "\n};\n\n";
  cpp << "} // namespace HPHP\n";

  return 0;
}
