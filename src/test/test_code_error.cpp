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

#include <test/test_code_error.h>
#include <compiler/parser/parser.h>
#include <compiler/builtin_symbols.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/code_generator.h>
#include <compiler/analysis/dependency_graph.h>
#include <compiler/option.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

TestCodeError::TestCodeError() {
  Option::IncludeRoots["$_SERVER['PHP_ROOT']"] = "";
}

bool TestCodeError::RunTests(const std::string &which) {
  bool ret = true;
#define CODE_ERROR_ENTRY(x) RUN_TEST(Test ## x);
#include "../compiler/analysis/core_code_error.inc"
#undef CODE_ERROR_ENTRY
  return ret;
}

bool TestCodeError::Verify(CodeError::ErrorType type, const char *src,
                           const char *file, int line, bool exists) {
  AnalysisResultPtr ar(new AnalysisResult());
  Parser::ParseString("<?php ", ar, "f2"); // for TestPHPIncludeFileNotInLib
  Parser::ParseString(src, ar, "f1");
  BuiltinSymbols::Load(ar);
  ar->analyzeProgram();
  ar->inferTypes();
  CodeErrorPtr ce = ar->getCodeError();
  if (ce->exists(type) != exists) {
    ostringstream code;
    CodeGenerator cg(&code);
    ar->outputAllCPP(cg);
    ostringstream error;
    JSON::OutputStream(error) << ar->getCodeError();
    printf("%s:%d: parsing %s\ncode error missing\n%s\n", file, line, src,
           error.str().c_str());
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeError::TestBadPHPIncludeFile() {
  VE(BadPHPIncludeFile, "<?php include $BAD.'unknown.php';");
  VE(BadPHPIncludeFile, "<?php include_once $template_path;");
  return true;
}

bool TestCodeError::TestRedundantInclude() {
  VE(RedundantInclude, "<?php require('a'); require('a');");
  return true;
}

bool TestCodeError::TestUseInclude() {
  VE(UseInclude, "<?php include $_SERVER['PHP_ROOT'].'f2';");
  VE(UseInclude, "<?php include $template_path;");
  VE(UseInclude, "<?php require $_SERVER['PHP_ROOT'].'f2';");
  VE(UseInclude, "<?php require $template_path;");
  return true;
}

bool TestCodeError::TestPHPIncludeFileNotFound() {
  VE(PHPIncludeFileNotFound,  "<?php include $_SERVER['PHP_ROOT'].'a.php';");
  VEN(PHPIncludeFileNotFound, "<?php include_once $template_path;");
#ifdef HPHP_NOTE
  VEN(PHPIncludeFileNotFound, "<?php /*|@PHPIncludeFileNotFound|*/ include $_SERVER['PHP_ROOT'].'a.php';");
#endif
  return true;
}

bool TestCodeError::TestPHPIncludeFileNotInLib() {
  VE(PHPIncludeFileNotInLib, "<?php include $_SERVER['PHP_ROOT'].'f2';");
  return true;
}

bool TestCodeError::TestUseDynamicInclude() {
  VE(UseDynamicInclude, "<?php if (true) include $_SERVER['PHP_ROOT'].'f2';");
  return true;
}

bool TestCodeError::TestUseLDynamicVariable() {
  VE(UseLDynamicVariable, "<?php $$a = 1;");
  return true;
}

bool TestCodeError::TestUseRDynamicVariable() {
  VE(UseRDynamicVariable, "<?php print $$a;");
  return true;
}

bool TestCodeError::TestUseDynamicFunction() {
  VE(UseDynamicFunction, "<?php $$a();");
  return true;
}

bool TestCodeError::TestUseDynamicClass() {
  VE(UseDynamicClass, "<?php new $$a();");
  return true;
}

bool TestCodeError::TestUseDynamicProperty() {
  VE(UseDynamicProperty, "<?php $obj->$a = 1;");
  return true;
}

bool TestCodeError::TestUseDynamicMethod() {
  VE(UseDynamicMethod, "<?php $obj->$a();");
  return true;
}

bool TestCodeError::TestUseDynamicGlobal() {
  VE(UseDynamicGlobal, "<?php function t() { global $$a;}");
  return true;
}

bool TestCodeError::TestUseEvaluation() {
  VE(UseEvaluation, "<?php eval('a');");
  return true;
}

bool TestCodeError::TestUseExtract() {
  VE(UseExtract, "<?php extract($a);");
  return true;
}

bool TestCodeError::TestUseShellExec() {
  VE(UseShellExec, "<?php shell_exec('test');");
  VE(UseShellExec, "<?php `test`;");
  return true;
}

bool TestCodeError::TestUseNotSupportedUnset() {
  VE(UseNotSupportedUnset,  "<?php unset($$b);");
  return true;
}

bool TestCodeError::TestUseUndeclaredVariable() {
  VE(UseUndeclaredVariable, "<?php print $a;");
  VE(UseUndeclaredVariable, "<?php $a = 1; function t() { print $a;}");
  VE(UseUndeclaredVariable, "<?php class T {} function t() { $a = new T(); print $a->a; }");
  //VE(UseUndeclaredVariable, "<?php print $GLOBALS['a'];");
  return true;
}

bool TestCodeError::TestPossibleUndeclaredVariable() {
  VE(PossibleUndeclaredVariable, "<?php while(true) { print $a; $a = 1;}");
  return true;
}

bool TestCodeError::TestUseUndeclaredConstant() {
  VE(UseUndeclaredConstant, "<?php print a;");
  VE(UseUndeclaredConstant, "<?php class T {} print T::a;");
  return true;
}

bool TestCodeError::TestUnknownClass() {
  VE(UnknownClass, "<?php $a = new T();");
  return true;
}

bool TestCodeError::TestUnknownBaseClass() {
  VE(UnknownBaseClass, "<?php class T extends R {}");
  return true;
}

bool TestCodeError::TestUnknownObjectMethod() {
  VE(UnknownObjectMethod, "<?php class T {} $a = new T(); print $a->a();");

  // negatve cases
  VEN(UnknownObjectMethod,
      "<?php class T { function __call($name, $args) {} } "
      "function t() { $a = new T(); print $a->a(); }");
  VEN(UnknownObjectMethod,
      "<?php class T { function __call($name, $args) {}} class R extends T {} "
      "function test(R $a) { print $a->a();}");
  return true;
}

bool TestCodeError::TestDerivedObjectMethod() {
  VE(DerivedObjectMethod,
     "<?php class T { function test() { $this->a();}} "
     "class R extends T { function a() {}}");
  return true;
}

bool TestCodeError::TestUnknownMagicMethod() {
  VE(UnknownMagicMethod,
     "<?php class T { function __a() {}}");
  return true;
}

bool TestCodeError::TestInvalidMagicMethod() {
  VE(InvalidMagicMethod,
     "<?php class T { function __tostring($a) {}}");
  return true;
}

bool TestCodeError::TestUnknownFunction() {
  VE(UnknownFunction, "<?php test();");
  return true;
}

bool TestCodeError::TestBadConstructorCall() {
  VE(BadConstructorCall, "<?php class T {} $a = new T(1);");

  // negative cases
  VEN(BadConstructorCall,
      "<?php class B { function __construct($a) {}} "
      "class A extends B {} $a = new A(1);");

  return true;
}

bool TestCodeError::TestDeclaredClassTwice() {
  VE(DeclaredClassTwice, "<?php class T {} class T {}");
  return true;
}

bool TestCodeError::TestDeclaredFunctionTwice() {
  VE(DeclaredFunctionTwice, "<?php function t() {} function t() {}");
  return true;
}

bool TestCodeError::TestDeclaredVariableTwice() {
  VE(DeclaredVariableTwice, "<?php class T { var $a; var $a;}");
  VE(DeclaredVariableTwice, "<?php class T { var $a = 1; var $a;}");
  VE(DeclaredVariableTwice, "<?php class T { var $a; var $a = 1;}");
  VE(DeclaredVariableTwice, "<?php class T { var $a = 1; var $a = 2;}");
  VE(DeclaredVariableTwice, "<?php class T { static $a; static $a;}");
  VE(DeclaredVariableTwice, "<?php class T { static $a = 1; static $a;}");
  VE(DeclaredVariableTwice, "<?php class T { static $a; static $a = 1;}");
  VE(DeclaredVariableTwice, "<?php class T { static $a = 1; static $a = 2;}");
  VE(DeclaredVariableTwice, "<?php class T { var $a; static $a;}");
  VE(DeclaredVariableTwice, "<?php class T { static $a; var $a;}");
  VE(DeclaredVariableTwice, "<?php class T { var $a = 1; static $a;}");
  VE(DeclaredVariableTwice, "<?php class T { var $a; static $a = 1;}");
  VE(DeclaredVariableTwice, "<?php class T { var $a = 1; static $a = 2;}");
  VE(DeclaredVariableTwice, "<?php function t() { static $a, $a;}");
  VE(DeclaredVariableTwice, "<?php function t() { global $a, $a;}");
  VE(DeclaredVariableTwice, "<?php function t() { global $a; global $a;}");
  VE(DeclaredVariableTwice, "<?php { global $a; global $a;}");
  VE(DeclaredVariableTwice, "<?php { static $a; static $a;}");
  VE(DeclaredVariableTwice, "<?php { global $a; static $a;}");
  VE(DeclaredVariableTwice, "<?php { static $a; global $a;}");

  // negative cases
  VEN(DeclaredVariableTwice, "<?php function t() { $a = 1; $a = 2;}");
  VEN(DeclaredVariableTwice,
      "<?php "
      "function s() { global $a; $a = 1;}"
      "function t() { global $a; $a = 1;}");
  return true;
}

bool TestCodeError::TestDeclaredConstantTwice() {
  VE(DeclaredConstantTwice, "<?php define('t', 1); define('t', 2);");
  VE(DeclaredConstantTwice, "<?php class T { const A = 1; const A = 1;}");
  return true;
}

bool TestCodeError::TestDeclaredStaticVariableTwice() {
  VE(DeclaredStaticVariableTwice,
     "<?php function a() { static $a; if ($b) { static $a = 1; } } ");
  VE(DeclaredStaticVariableTwice,
     "<?php function test() { static $a; if ($b) { static $a = 1; } } ");
  return true;
}

bool TestCodeError::TestBadDefine() {
  VE(BadDefine, "<?php define($a, 1);");
  return true;
}

bool TestCodeError::TestRequiredAfterOptionalParam() {
  VE(RequiredAfterOptionalParam, "<?php function t($a = 1, $b) {}");
  return true;
}

bool TestCodeError::TestRedundantParameter() {
  VE(RedundantParameter, "<?php function t($a, $a) {}");
  return true;
}

bool TestCodeError::TestTooFewArgument() {
  VE(TooFewArgument, "<?php function test($a) {} test();");
  VE(TooFewArgument, "<?php function test($a, $b) {} test(1);");
  VE(TooFewArgument,
     "<?php class T { function t($a) {}} $a = new T(); $a->t();");
  VE(TooFewArgument,
     "<?php class T { function t($a, $b) {}} $a = new T(); $a->t(1);");
  VE(TooFewArgument,
     "<?php class T { function __construct($a) {}} $a = new T();");
  VE(TooFewArgument,
     "<?php class T { function __construct($a, $b) {}} $a = new T(1);");
  return true;
}

bool TestCodeError::TestTooManyArgument() {
  VE(TooManyArgument, "<?php function test() {} test(1);");
  VE(TooManyArgument,
     "<?php class A { function t() {}} "
     "function test() { $a = new A(); $a->t(1);}");
  VE(TooManyArgument,
     "<?php class T { function __construct($b) {}} $a = new T(1, 2);");

  // negative cases
  VEN(TooManyArgument,
      "<?php function t() {} function t($a) {} t($a);");
  return true;
}

bool TestCodeError::TestBadTypeConversion() {
  VE(BadTypeConversion, "<?php clone(1);");
  VE(BadTypeConversion, "<?php class T { const a = 'test'; } T::a + 1;");

  // negative cases
  VEN(BadTypeConversion,
      "<?php function t() { return array(0);} while($a = t()) {print $a[1];}");
  VEN(BadTypeConversion, "<?php while($a = test()) { print $a[1];}");
  VEN(BadTypeConversion, "<?php function t($a = 1) { print $a;} t(true);");
  VEN(BadTypeConversion, "<?php function t() {return array(); return false;}");

  return true;
}

bool TestCodeError::TestComplexForEach() {
  VE(ComplexForEach, "<?php function test() {} foreach(test() as $a) {}");
  return true;
}

bool TestCodeError::TestStatementHasNoEffect() {
  VE(StatementHasNoEffect, "<?php $a;");
  VE(StatementHasNoEffect, "<?php $a[$b + $c];");
  VE(StatementHasNoEffect, "<?php $a + $b;");
  VE(StatementHasNoEffect, "<?php 'test';");
  VE(StatementHasNoEffect, "<?php -$a;");
  return true;
}

bool TestCodeError::TestBadReturnStatement() {
  // we don't have a case yet
  return true;
}

bool TestCodeError::TestUseVoidReturn() {
  VE(UseVoidReturn, "<?php function test() {} $a = test();");
  return true;
}

bool TestCodeError::TestMissingObjectContext() {
  VE(MissingObjectContext,
     "<?php class A { public $a = 123; "
     "public static function test() { print $this->a;}} A::test();");

  VE(MissingObjectContext,
     "<?php class A { public function a() { } "
     "public static function test() { $this->a();}} A::test();");

  VE(MissingObjectContext,
     "<?php class A { public $a = 123; } print A::$a;");

  VE(MissingObjectContext,
     "<?php class A { public function a() {} } print A::a();");

  VE(MissingObjectContext,
     "<?php class A { public function a() {} } "
     "class B { public function b() { A::a();}}");

  // negative case
  VEN(MissingObjectContext,
      "<?php class A { public function a() {} } "
      "class B extends A { public function b() { A::a();}}");

  return true;
}

bool TestCodeError::TestCaseAfterDefault() {
  VE(CaseAfterDefault, "<?php switch ($a) { default: case 1: }");
  return true;
}

bool TestCodeError::TestMoreThanOneDefault() {
  VE(MoreThanOneDefault, "<?php switch ($a) { default: default: }");
  return true;
}

bool TestCodeError::TestInvalidArrayElement() {
  VE(InvalidArrayElement, "<?php if (isset($obj[])) var_dump($obj);");
  return true;
}

bool TestCodeError::TestInvalidDerivation() {
  VE(InvalidDerivation,
     "<?php "
     "interface RecurisiveFooFar extends RecurisiveFooFar {}");
  VE(InvalidDerivation,
     "<?php "
     "class RecurisiveFooFar extends RecurisiveFooFar {}");
  VE(InvalidDerivation,
     "<?php "
     "interface a extends b {}"
     "interface b extends a {}");
  VE(InvalidDerivation,
     "<?php "
     "interface a extends B {}"
     "interface b extends a {}");
  VE(InvalidDerivation,
     "<?php "
     "interface a extends b {}"
     "interface B extends a {}");
  VE(InvalidDerivation,
     "<?php "
     "interface a extends b {}"
     "interface b extends A {}");
  VE(InvalidDerivation,
     "<?php "
     "interface A extends B {}"
     "interface b extends a {}");
  VE(InvalidDerivation,
     "<?php "
     "interface a extends B {}"
     "interface B extends a {}");
  VE(InvalidDerivation,
     "<?php "
     "interface a extends b {}"
     "interface B extends A {}");
  VE(InvalidDerivation,
     "<?php "
     "interface A extends B {}"
     "interface B extends a {}");
  VE(InvalidDerivation,
     "<?php "
     "interface a extends B {}"
     "interface B extends A {}");
  VE(InvalidDerivation,
     "<?php "
     "class a extends b {}"
     "class B extends A {}");
  VE(InvalidDerivation,
     "<?php "
     "interface a extends b {}"
     "class B extends A {}");
  VE(InvalidDerivation, "<?php interface A {} class T implements A, A {}");
  VE(InvalidDerivation, "<?php class A {} class B implements A {}");
  VE(InvalidDerivation, "<?php class A {} interface B extends A {}");
  VEN(InvalidDerivation,
      "<?php "
      "class A {} "
      "interface B {} "
      "class C extends A  implements B {}");
  VE(InvalidDerivation, "<?php interface I {} class C extends I {}");

  return true;
}

bool TestCodeError::TestReassignThis() {
  VE(ReassignThis,
     "<?php "
     "class Foo {"
     "  function Bar() {"
     "    $__this = $this;"
     "    $this = null;"
     "    debug_backtrace();"
     "    $this = $__this;"
     "  }"
     "}");

  return true;
}

bool TestCodeError::TestMissingAbstractMethodImpl() {
  VE(MissingAbstractMethodImpl,
     "<?php "
     "abstract class A {"
     "  abstract function foo();"
     "}"
     "class B extends A {"
     "}");
  VE(MissingAbstractMethodImpl,
     "<?php "
     "interface A {"
     "  function foo();"
     "}"
     "class B implements A {"
     "}");
  VE(MissingAbstractMethodImpl,
     "<?php "
     "if (true) {"
     "  abstract class A {"
     "    abstract function foo();"
     "  }"
     "} else {"
     "  abstract class A {"
     "    abstract function foo();"
     "  }"
     "}"
     "class B extends A {"
     "}");
  VEN(MissingAbstractMethodImpl,
      "interface A {"
      "  public function foo();"
      "}"
      "class B implements A {"
      "  public function foo() {"
      "    return;"
      "  }"
      "}"
      "interface C extends A { }"
      "class D extends B implements C {}"
      "class E extends D { }");

  return true;
}

bool TestCodeError::TestBadPassByReference() {
  VE(BadPassByReference,
     "<?php "
     "function set_to_null(&$i) { $i = null; }"
     "set_to_null(1);");
  VE(BadPassByReference,
     "<?php "
     "function set_to_null(&$i) { $i = null; }"
     "class A { const C  = 1; }"
     "set_to_null(A::C);");
  VE(BadPassByReference,
     "<?php "
     "function set_to_null(&$i) { $i = null; }"
     "set_to_null($a + $b);");
  VE(BadPassByReference,
     "<?php "
     "function set_to_null(&$i) { $i = null; }"
     "set_to_null(foo() + foo());");
  VE(BadPassByReference,
     "<?php "
     "function set_to_null(&$i) { $i = null; }"
     "set_to_null(array(1));");
  VE(BadPassByReference,
     "<?php "
     "function set_to_null(&$i) { $i = null; }"
     "define('A', 1);"
     "set_to_null(A);");
  VE(BadPassByReference,
     "<?php "
     "function set_to_null(&$i) { $i = null; }"
     "set_to_null($a ? $b : $c);");
  VE(BadPassByReference,
     "<?php "
     "class A { function foo(&$a) { echo $a;} }"
     "class B { function bar() { $obj = new A; $obj->foo(1);  } }");

  VEN(BadPassByReference,
      "<?php "
      "function set_to_null(&$i) { $i = null; }"
      "function foo() { return 1; }"
      "class A { var $m; static $n; function f() { return 1;} }"
      "set_to_null($a);"
      "set_to_null($a = 1);"
      "set_to_null(($a = 1));"
      "set_to_null(new A);"
      "set_to_null(foo());"
      "$a = 'foo';"
      "$b = 'a';"
      "set_to_null($a());"
      "set_to_null($$b);"
      "$i = 1;"
      "set_to_null(++$i); set_to_null($i--);"
      "set_to_null(--$i); set_to_null($i--);"
      "$obj = new A;"
      "set_to_null($obj->f());"
      "set_to_null($obj->m);"
      "set_to_null(A::$n);");
  VEN(BadPassByReference,
      "$ar = array("
      "       array('10', 11, 100, 100, 'a'),"
      "       array(   1,  2, '2',   3,   1)"
      "      );"
      "array_multisort($ar[0], SORT_ASC, SORT_STRING,"
      "                $ar[1], SORT_NUMERIC, SORT_DESC);");
  return true;
}
