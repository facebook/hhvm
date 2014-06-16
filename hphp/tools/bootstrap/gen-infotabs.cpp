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

/// The set of PHP class names which need to be initialised before systemlib.php
const fbstring baseClasses[] = {
  fbstring("Closure")
};

void print_classes(std::ostream& cpp, const char* identifier,
                   const fbvector<PhpClass>& classes,
                   const std::unordered_set<fbstring>& classesWithCtors);

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
    fbstring name = func.lowerCppName();
    cpp << "TypedValue* fg_" << name << "(ActRec* ar);\n";
  }

  for (auto const& klass : classes) {
    if (!(klass.flags() & IsCppAbstract)) {
      cpp << "ObjectData* new_" << klass.getCppName() << "_Instance(Class*);\n";
      classesWithCtors.insert(klass.getCppName());
    }
    for (auto const& func : klass.methods()) {
      cpp << "TypedValue* tg_" << func.getUniqueName()
          << "(ActRec* ar);\n";
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
    fbstring name = func.lowerCppName();
    cpp << "{ \"" << escapeCpp(func.getPhpName()) << "\", " << "fg_" << name
        << ", (void *)&" << prefix << name << " }";
  }
  cpp << "\n};\n\n";

  for (auto const& klass : classes) {
    cpp << "static const long long hhbc_ext_method_count_"
        << klass.getCppName() << " = " << klass.numMethods()
        << ";\n"
        << "static const HhbcExtMethodInfo hhbc_ext_methods_"
        << klass.getCppName() << "[] = {\n  ";
    first = true;
    for (auto const& method : klass.methods()) {
      if (!first) {
        cpp << ",\n  ";
      }
      first = false;

      auto name = method.getUniqueName();
      cpp << "{ \"" << method.getCppName() << "\", "
          << "tg_" << name << ", "
          << "(void*)&th_" << name << " }";
    }
    cpp << "\n};\n\n";
  }

  // We need to separate the list of classes out. Some classes need to be loaded
  // before the systemlib.php gets loaded (e.g. Closure). The rest will be
  // loaded *after* systemlib.php
  fbvector<PhpClass> baseClasses;
  {
    std::unordered_set<fbstring> baseClassNames;
    std::for_each(::baseClasses, ::baseClasses + sizeof(::baseClasses),
                  [&baseClassNames] (const fbstring& klass) {
      baseClassNames.insert(klass);
    });

    // Move all base classes to the baseClasses vector, and store the iterator
    // to that element so we can remove it from the classes vector.
    fbvector<fbvector<PhpClass>::iterator> baseClassIterators;
    for (auto i = begin(classes), j = end(classes); i != j; ++i) {
      if (baseClassNames.find(i->getPhpName()) != end(baseClassNames)) {
        baseClasses.emplace_back(*i);
        baseClassIterators.push_back(i);
      }
    }

    // Filter out the base classes out of the main classes vector
    for (auto i : baseClassIterators) {
      classes.erase(i);
    }
  }

  print_classes(cpp, "hhbc_ext_base_classes", baseClasses, classesWithCtors);
  print_classes(cpp, "hhbc_ext_classes", classes, classesWithCtors);

  cpp << "} // namespace HPHP\n";
  return 0;
}

void print_classes(std::ostream& cpp, const char* identifier,
                   const fbvector<PhpClass>& classes,
                   const std::unordered_set<fbstring>& classesWithCtors) {
  cpp << "const long long " << identifier << "_count = " << classes.size() << ";\n";

  for (auto& klass : classes) {
    cpp << "extern void "
        << folly::to<std::string>("delete_", klass.getCppName())
        << "(ObjectData*, const Class*);\n";
  }

  cpp << "const HhbcExtClassInfo " << identifier << "[] = {\n  ";
  bool first = true;
  for (auto const& klass : classes) {
    if (!first) {
      cpp << ",\n  ";
    }
    first = false;

    auto ctor = classesWithCtors.count(klass.getCppName()) > 0
                 ? fbstring("new_") + klass.getCppName() + "_Instance"
                 : fbstring("nullptr");

    auto dtor = classesWithCtors.count(klass.getCppName()) > 0
                 ? folly::to<std::string>("delete_", klass.getCppName())
                 : fbstring{"nullptr"};

    auto cpp_name = klass.getCppName();

    auto const c_cpp_name = "c_" + cpp_name;
    cpp << "{ \"" << escapeCpp(klass.getPhpName()) << "\", " << ctor
        << "," << dtor
        << ", sizeof(" << c_cpp_name << ')'
        << ", intptr_t("
               "static_cast<ObjectData*>("
                 "reinterpret_cast<" << c_cpp_name << "*>(0x100)"
               ")"
             ") - 0x100"
        << ", hhbc_ext_method_count_" << klass.getCppName()
        << ", hhbc_ext_methods_" << klass.getCppName()
        << ", &" << c_cpp_name << "::classof() }";
  }
  cpp << "\n};\n\n";
}
