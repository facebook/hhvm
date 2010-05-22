/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <test/test_depend_graph.h>
#include <compiler/parser/parser.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/code_error.h>
#include <compiler/code_generator.h>
#include <compiler/option.h>
#include <compiler/builtin_symbols.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

bool TestDependGraph::RunTests(const std::string &which) {
  bool ret = true;
#define DEPENDENCY_ENTRY(x) RUN_TEST(Test ## x);
#include "../compiler/analysis/core_dependency.inc"
#undef DEPENDENCY_ENTRY
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestDependGraph::VerifyDependency(DependencyGraph::KindOf kindOf,
                                       const char *input1,
                                       const char *input2 /* = NULL */,
                                       const char *parent /* = "f2" */,
                                       const char *child /* = "f1" */,
                                       const char *file /* = "" */,
                                       int line /* = 0 */) {
  ASSERT(input1);

  Option::IncludeRoots["$_SERVER['PHP_ROOT']"] = "";
  AnalysisResultPtr ar(new AnalysisResult());
  BuiltinSymbols::Load(ar);
  Parser::ParseString(input1, ar, "f1");
  if (input2) Parser::ParseString(input2, ar, "f2");
  ar->analyzeProgram();
  ar->inferTypes();
  DependencyGraphPtr dg = ar->getDependencyGraph();
  const StringToConstructPtrMap &children = dg->getAllChildren(kindOf, parent);
  if ((*child && children.find(child) == children.end()) ||
      (!*child && !children.empty())) {
    ostringstream graph;
    JSON::OutputStream(graph) << ar->getDependencyGraph();
    printf("%s:%d: dependency missing\n%s\n", file, line,
           graph.str().c_str());
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestDependGraph::TestPHPInclude() {
  VD2(PHPInclude, "<?php include         $_SERVER['PHP_ROOT'].'f2';",  "");
  VD2(PHPInclude, "<?php include_once    $_SERVER['PHP_ROOT'].'f2';",  "");
  VD2(PHPInclude, "<?php require         $_SERVER['PHP_ROOT'].'f2';",  "");
  VD2(PHPInclude, "<?php require_once    $_SERVER['PHP_ROOT'].'f2';",  "");
  return true;
}

bool TestDependGraph::TestClassDerivation() {
  VD1(ClassDerivation, "<?php class f2 {} class f1 extends f2 {}");
  return true;
}

bool TestDependGraph::TestFunctionCall() {
  // functions
  VD4(FunctionCall,
      "<?php include $_SERVER['PHP_ROOT'].'f2';\n test();",
      "<?php function test() {}",
      "test", "test()");
  VD4(FunctionCall,
      "<?php test(); function test() {}", "", "test", "test()");
  VD4(FunctionCall,
      "<?php function test() {} test();", "", "test", "test()");

  // methods
  VD4(FunctionCall,
      "<?php include $_SERVER['PHP_ROOT'].'f2';\n"
      "function test() { $a = new C(); $a->test();}",
      "<?php class C { public function test() {}}",
      "c::test", "$a->test()");
  VD4(FunctionCall,
      "<?php function test() { $a = new C(); $a->test();} "
      "class C { public function test() {}}",
      "", "c::test", "$a->test()");
  VD4(FunctionCall,
      "<?php class C { public function test() {}} "
      "function test() { $a = new C(); $a->test();}",
      "", "c::test", "$a->test()");

#ifdef HPHP_NOTE
  // making sure a "MasterCopy" marked function will always be used for
  // dependency's parent
  {
    Option::IncludeRoots["$_SERVER['PHP_ROOT']"] = "";
    AnalysisResultPtr ar(new AnalysisResult());
    BuiltinSymbols::Load(ar);
    Parser::ParseString("<?php function test() {}", ar, "f1");
    Parser::ParseString("<?php /*|MasterCopy|*/ function test() {}", ar, "f2");
    ar->analyzeProgram();
    ar->inferTypes();
    DependencyGraphPtr dg = ar->getDependencyGraph();
    ConstructPtr parent = dg->getParent(DependencyGraph::KindOfFunctionCall,
                                        "test");
    if (!parent || strcmp(parent->getLocation()->file, "f2") != 0) {
      printf("%s:%d: incorrect parent found at %s:%d\n", __FILE__, __LINE__,
             parent ? parent->getLocation()->file : "",
             parent ? parent->getLocation()->line0 : 0);
      return false;
    }
  }
#endif

  return true;
}

bool TestDependGraph::TestGlobalVariable() {
  VD4(GlobalVariable,
      "<?php $a = 1;\nfunction test() { global $a;}",
      "", "a", "a");
  VD4(GlobalVariable,
      "<?php $GLOBALS['a'] = 1;\nfunction test() { global $a;}",
      "", "a", "a");
  VD4(GlobalVariable,
      "<?php $a = 1;\nfunction test() { $GLOBALS['a'] = 2;}",
      "", "a", "a");
  VD4(GlobalVariable,
      "<?php $a = 1;\nfunction test() { $GLOBALS['a']['b'] = 2;}",
      "", "a", "a");
  VD4(GlobalVariable,
      "<?php $a = 1;\nfunction test() { $c->$GLOBALS['a'] = 2;}",
      "", "a", "a");
  VD4(GlobalVariable,
      "<?php $a = 1;\nfunction test() { $c->GLOBALS['a'] = 2;}",
      "", "a", "");
  return true;
}

bool TestDependGraph::TestConstant() {
  // global constants
  VD4(Constant,
      "<?php define('TEST', 1); $a = TEST;", "", "TEST", "TEST");

  // class constants
  VD4(Constant,
      "<?php class A { const B = 1;} $a = A::B;", "", "a::B", "a::B");

  return true;
}

bool TestDependGraph::TestProgramMaxInclude() {
  // TODO
  return true;
}

bool TestDependGraph::TestProgramMinInclude() {
  // TODO
  return true;
}

bool TestDependGraph::TestProgramUserFunction() {
  // TODO
  return true;
}

bool TestDependGraph::TestProgramUserClass() {
  // TODO
  return true;
}
