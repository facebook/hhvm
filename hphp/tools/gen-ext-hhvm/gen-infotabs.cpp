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

#include "idl.h"

using folly::fbstring;
using folly::fbvector;


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

  cpp << "#include \"runtime/ext_hhvm/ext_hhvm.h\"\n";
  cpp << "#include \"runtime/ext/ext.h\"\n";
  cpp << "#include \"ext_hhvm_infotabs.h\"\n";
  cpp << "namespace HPHP {\n";
  cpp << "  struct TypedValue;\n";
  cpp << "  struct ActRec;\n";
  cpp << "  namespace VM { struct Class; }\n";
  cpp << "}\n\n";

  cpp << "namespace HPHP {\n";

  std::unordered_set<fbstring> classesWithCtors;

  ////////////////////////////////
  // Declare the fg_ and tg_ stubs

  for (auto const& func : funcs) {
    cpp << "TypedValue* fg_" << func.name << "(ActRec* ar);\n";
  }

  for (auto const& klass : classes) {
    if (std::find(klass.flags.begin(), klass.flags.end(), "IsCppAbstract")
        == klass.flags.end()) {
      cpp << "VM::Instance* new_" << klass.name << "_Instance(VM::Class*);\n";
      classesWithCtors.insert(klass.name);
    }
    for (auto const& func : klass.methods) {
      cpp << "TypedValue* tg_" << func.getUniqueName()
          << "(ActRec* ar);\n";
    }
  }

  cpp << "\n";

  ///////////////////////////////////////
  // Define the name - fg_ - fh_ mappings

  cpp << "const long long hhbc_ext_funcs_count = " << funcs.size() << ";\n";
  cpp << "const HhbcExtFuncInfo hhbc_ext_funcs[] = {\n  ";

  bool first = true;
  for (auto const& func : funcs) {
    if (!func.className.empty()) {
      continue;
    }
    if (!first) {
      cpp << ",\n  ";
    }
    first = false;

    cpp << "{ \"" << func.name << "\", fg_" << func.name
        << ", (void *)&fh_" << func.name << " }";
  }
  cpp << "\n};\n\n";

  for (auto const& klass : classes) {
    cpp << "static const long long hhbc_ext_method_count_" << klass.name
        << " = " << klass.methods.size() << ";\n";
    cpp << "static const HhbcExtMethodInfo hhbc_ext_methods_" << klass.name
        << "[] = {\n  ";
    first = true;
    for (auto const& method : klass.methods) {
      if (!first) {
        cpp << ",\n  ";
      }
      first = false;

      cpp << "{ \"" << method.name << "\", tg_" << method.getUniqueName()
          << " }";
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

    auto ctor = (classesWithCtors.count(klass.name) > 0
                 ? fbstring("new_") + klass.name + "_Instance"
                 : fbstring("nullptr"));

    cpp << "{ \"" << klass.name << "\", " << ctor
        << ", sizeof(c_" << klass.name << ')'
        << ", hhbc_ext_method_count_" << klass.name
        << ", hhbc_ext_methods_" << klass.name
        << ", &c_" << klass.name << "::s_cls }";
  }
  cpp << "\n};\n\n";
  cpp << "} // namespace HPHP\n";

  return 0;
}
