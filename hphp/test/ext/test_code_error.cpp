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

#include "hphp/test/ext/test_code_error.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/code_generator.h"
#include "hphp/compiler/option.h"

///////////////////////////////////////////////////////////////////////////////

TestCodeError::TestCodeError() {
  Option::IncludeRoots["$_SERVER['PHP_ROOT']"] = "";
}

bool TestCodeError::RunTests(const std::string &which) {
  bool ret = true;
#define CODE_ERROR_ENTRY(x) RUN_TEST(Test ## x);
#include "hphp/compiler/analysis/core_code_error.inc"
#undef CODE_ERROR_ENTRY
  return ret;
}

bool TestCodeError::Verify(Compiler::ErrorType type, const char *src,
                           const char *file, int line, bool exists) {
  WithOpt w0(Option::RecordErrors);
  WithOpt w1(Option::WholeProgram);
  WithOpt w2(Option::ParseTimeOpts);

  Compiler::ClearErrors();

  Type::ResetTypeHintTypes();
  Type::InitTypeHintMap();
  BuiltinSymbols::LoadSuperGlobals();

  AnalysisResultPtr ar(new AnalysisResult());
  // for TestPHPIncludeFileNotInLib
  Compiler::Parser::ParseString("<?php ", ar, "f2");
  Compiler::Parser::ParseString(src, ar, "f1");
  BuiltinSymbols::Load(ar);
  ar->analyzeProgram();
  ar->inferTypes();
  ar->analyzeProgramFinal();
  if (Compiler::HasError(type) != exists) {
    std::ostringstream error;
    JSON::CodeError::OutputStream out(error, ar);
    Compiler::SaveErrors(out);
    printf("%s:%d: parsing %s\ncode error missing\n%s\n", file, line, src,
           error.str().c_str());
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeError::TestBadPHPIncludeFile() {
  VE(BadPHPIncludeFile, "<?php include 'f1';");
  return true;
}

bool TestCodeError::TestPHPIncludeFileNotFound() {
  VE(PHPIncludeFileNotFound,  "<?php include $_SERVER['PHP_ROOT'].'a.php';");
  VEN(PHPIncludeFileNotFound, "<?php include_once $template_path;");
  return true;
}

bool TestCodeError::TestUseEvaluation() {
  VE(UseEvaluation, "<?php eval('a');");
  return true;
}

bool TestCodeError::TestUseUndeclaredVariable() {
  VE(UseUndeclaredVariable, "<?php $a = 1; function t() { print $a;}");
/*
  Removing for now. We dont warn about non-static properties.

  VE(UseUndeclaredVariable,
     "<?php class T {} function t() { $a = new T(); print $a->a; }");
*/
  VE(UseUndeclaredVariable,
     "<?php class A { public $a = 123; } print A::$a;");

  return true;
}

bool TestCodeError::TestUseUndeclaredGlobalVariable() {
  VE(UseUndeclaredGlobalVariable, "<?php print $a;");
  VE(UseUndeclaredGlobalVariable, "<?php print $GLOBALS['a'];");
  return true;
}

bool TestCodeError::TestUseUndeclaredConstant() {
  VE(UseUndeclaredConstant, "<?php print a;");
  VE(UseUndeclaredConstant, "<?php class T {} print T::a;");
  return true;
}

bool TestCodeError::TestUnknownClass() {
  VE(UnknownClass, "<?php $a = new T();");

  VEN(UnknownClass, "<?php class A { function foo(self $a) {}}");
  VEN(UnknownClass,
      "<?php "
      "trait A {"
      "  function foo($f) {"
      "    echo self::FOO+parent::FOO;"
      "    echo self::$f()+parent::$f();"
      "    var_dump(new self, new parent);"
      "    echo self::bar()+parent::bar();"
      "    echo self::$foo+parent::$foo;"
      "    try {} catch (self $p) {}"
      "    try {} catch (parent $p) {}"
      "  }"
      "}");
  return true;
}

bool TestCodeError::TestUnknownBaseClass() {
  VE(UnknownBaseClass, "<?php class T extends R {}");
  return true;
}

bool TestCodeError::TestUnknownObjectMethod() {
  VE(UnknownObjectMethod,
     "<?php class T {} "
     "function test() { $a = new T(); print $a->a(); }");

  // negatve cases
  VEN(UnknownObjectMethod,
      "<?php class T { function __call($name, $args) {} } "
      "function t() { $a = new T(); print $a->a(); }");
  VEN(UnknownObjectMethod,
      "<?php class T { function __call($name, $args) {}} class R extends T {} "
      "function test(R $a) { print $a->a();}");
  return true;
}

bool TestCodeError::TestInvalidMagicMethod() {
  VE(InvalidMagicMethod, "<?php class T { function __tostring($a) {}}");
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
  return true;
}

bool TestCodeError::TestDeclaredConstantTwice() {
  VE(DeclaredConstantTwice, "<?php define('t', 1); define('t', 2);");
  VE(DeclaredConstantTwice, "<?php class T { const A = 1; const A = 1;}");
  return true;
}

bool TestCodeError::TestDeclaredMethodTwice() {
  VE(DeclaredMethodTwice, "<?php class T { function foo(){} function foo(){}}");
  return true;
}

bool TestCodeError::TestDeclaredAttributeTwice() {
  WithOpt w0(Option::EnableHipHopSyntax);

  VE(DeclaredAttributeTwice, "<?php << Foo, Foo >> class C {}");
  VE(DeclaredAttributeTwice, "<?php << Foo, Foo >> function f() {}");
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

bool TestCodeError::TestStatementHasNoEffect() {
  VE(StatementHasNoEffect, "<?php $a;");
  VE(StatementHasNoEffect, "<?php $a + $b;");
  VE(StatementHasNoEffect, "<?php 'test';");
  VE(StatementHasNoEffect, "<?php -$a;");
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

  // negative case
  VEN(MissingObjectContext,
      "<?php class A { public function a() {} } "
      "class B extends A { public function b() { A::a();}}");

  VEN(MissingObjectContext,
      "<?php class A { public function a() {} } print A::a();");

  VEN(MissingObjectContext,
      "<?php class A { public function a() {} } "
      "class B { public function b() { A::a();}}");

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

bool TestCodeError::TestInvalidOverride() {
  VE(InvalidOverride,
     "<?php class A { protected $x; } class B extends A { private $x; }");

  VE(InvalidOverride,
     "<?php class A { public $x; } class B extends A { private $x; }");

  VE(InvalidOverride,
     "<?php class A { public $x; } class B extends A { protected $x; }");

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
  VEN(MissingAbstractMethodImpl,
     "<?php "
     "if (true) {"
     "  abstract class A {"
     "    abstract function foo();"
     "  }"
     "} else {"
     "  abstract class A {"
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

bool TestCodeError::TestConditionalClassLoading() {
  VE(ConditionalClassLoading,
    "<?php "
    "function load_cls($x) { "
    "  if (class_exists($x)) { return; } "
    "  switch ($x) { "
    "  case 'C1': /* require_once somefile */ break; "
    "  case 'C2': /* require_once somefile */ break; "
    "  } "
    "} "
    "load_cls('C1');");
  return true;
}

bool TestCodeError::TestBadArgumentType() {
  VE(BadArgumentType,
     "<?php "
     "function foo(array $a) { }"
     "foo(0);");

  return true;
}

bool TestCodeError::TestGotoUndefLabel() {
  VE(GotoUndefLabel,
     "<?php goto foo_bar;");

  VE(GotoUndefLabel,
     "<?php function f() { goto baz; } baz:");

  return true;
}

bool TestCodeError::TestGotoInvalidBlock() {
  VE(GotoInvalidBlock,
     "<?php goto my_block; do { my_block: } while (false);");

  VE(GotoInvalidBlock,
     "<?php "
     "function f($x) {"
     "  goto foo;"
     "  for ($i = 0; $i < $x; $i++) {"
     "    foo: var_dump($i);"
     "  }"
     "}");

  return true;
}

bool TestCodeError::TestInvalidAttribute() {
  VE(InvalidAttribute,
     "<?php abstract class F { abstract $f; }");

  VE(InvalidAttribute,
     "<?php class F { abstract $f; }");

  VE(InvalidAttribute,
     "<?php final class F { final $f; }");

  VE(InvalidAttribute,
     "<?php class F { final $f; }");

  VE(InvalidAttribute,
     "<?php interface I { final function foo(); }");

  VE(InvalidAttribute,
     "<?php interface I { private function foo(); }");

  VE(InvalidAttribute,
     "<?php interface I { protected function foo(); }");

  VE(InvalidAttribute,
     "<?php interface I { abstract function foo(); }");

  VE(InvalidAttribute,
     "<?php class a { static function a() {} }");

  VE(InvalidAttribute,
     "<?php class a { static function __construct() {} }");

  VEN(InvalidAttribute,
      "<?php class a { function __construct() {} static function a() {} }");

  return true;
}

bool TestCodeError::TestUnknownTrait() {
  VE(UnknownTrait, "<?php class C { use T; }");

  VE(UnknownTrait, "<?php trait T1 { use T2; }");

  return true;
}

bool TestCodeError::TestMethodInMultipleTraits() {
  VE(MethodInMultipleTraits,
     "<?php\n"
     "trait T1 {\n"
     "  public function Func() { }\n"
     "}\n"
     "trait T2 {\n"
     "  public function Func() { }\n"
     "}\n"
     "class C {\n"
     "  use T1, T2;\n"
     "}\n");

  VE(MethodInMultipleTraits,
     "<?php\n"
     "trait T1 {\n"
     "  public function Func() { }\n"
     "}\n"
     "trait T2 {\n"
     "  public function Func() { }\n"
     "}\n"
     "trait T3 {\n"
     "  use T2;\n"
     "}\n"
     "class C {\n"
     "  use T1, T3;\n"
     "}\n");

  VE(MethodInMultipleTraits,
     "<?php\n"
     "trait T1 {\n"
     "  public function Func() { }\n"
     "}\n"
     "trait T2 {\n"
     "  use T1;\n"
     "}\n"
     "trait T3 {\n"
     "  use T1;\n"
     "}\n"
     "class C {\n"
     "  use T2, T3;\n"
     "}\n");

  VE(MethodInMultipleTraits,
     "<?php\n"
     "trait T1 {\n"
     "  public function Func1() { }\n"
     "}\n"
     "trait T2 {\n"
     "  public function Func2() { }\n"
     "}\n"
     "trait T3 {\n"
     "  use T2 {\n"
     "    T2::Func2 as Func1;\n"
     "  }\n"
     "}\n"
     "class C {\n"
     "  use T1, T3;\n"
     "}\n");

  return true;
}

bool TestCodeError::TestUnknownTraitMethod() {
  VE(UnknownTraitMethod,
     "<?php\n"
     "trait T1 {\n"
     "  public function Func1() { }\n"
     "}\n"
     "class C {\n"
     "  use T1 {\n"
     "    T1::Func2 as Func3;\n"
     "  }\n"
     "}\n");

  return true;
}

bool TestCodeError::TestCyclicDependentTraits() {
  VE(CyclicDependentTraits,
     "<?php\n"
     "trait T1 {\n"
     "  use T2;\n"
     "}\n"
     "trait T2 {\n"
     "  use T1;\n"
     "}\n");

  VE(CyclicDependentTraits,
     "<?php\n"
     "trait T1 {\n"
     "  use T1;\n"
     "}\n");

  VE(CyclicDependentTraits,
     "<?php\n"
     "trait T1 {\n"
     "  use T2;\n"
     "}\n"
     "trait T2 {\n"
     "  use T3;\n"
     "}\n"
     "trait T3 {\n"
     "  use T1;\n"
     "}\n");

  return true;
}

bool TestCodeError::TestInvalidTraitStatement() {
  VE(InvalidTraitStatement,
     "<?php\n"
     "trait T {\n"
     "  const y = 3;\n"
     "}\n");

  return true;
}

bool TestCodeError::TestRedeclaredTrait() {
  VE(RedeclaredTrait,
     "<?php\n"
     "trait T {}\n"
     "trait T {}\n");

  VE(RedeclaredTrait,
     "<?php\n"
     "trait T {}\n"
     "class T {}\n");

  VE(RedeclaredTrait,
     "<?php\n"
     "class T {}\n"
     "trait T {}\n");

  VE(RedeclaredTrait,
     "<?php\n"
     "interface T {}\n"
     "trait T {}\n");

  return true;
}

bool TestCodeError::TestInvalidInstantiation() {
  VE(InvalidInstantiation, "<?php interface T {}; $a = new T();");
  VE(InvalidInstantiation,
     "<?php abstract class T { abstract function foo(); };"
     "$a = new T();");

  VEN(InvalidInstantiation, "<?php class T {}; $a = new T();");
  return true;
}

bool TestCodeError::TestInvalidYield() {
  WithOpt w0(Option::EnableHipHopSyntax);

  VE(InvalidYield, "<?php function f() { yield 1; return 2; }");
  VE(InvalidYield, "<?php async function f() { await f(); yield 1; }");
  VE(InvalidYield, "<?php yield 1; }");
  VE(InvalidYield, "<?php class X { function __get() { yield 1; } }");
  VE(InvalidYield, "<?php class X { function X() { yield 1; } }");
  return true;
}


bool TestCodeError::TestInvalidAwait() {
  WithOpt w0(Option::EnableHipHopSyntax);

  VE(InvalidAwait, "<?php function f() { yield 1; await f(); }");
  VE(InvalidAwait, "<?php await f(); }");
  VE(InvalidAwait, "<?php class X { function __get() { await f(); } }");
  VE(InvalidAwait, "<?php class X { function X() { await f(); } }");
  return true;
}


bool TestCodeError::TestBadDefaultValueType() {
  WithOpt w0(Option::EnableHipHopSyntax);

  VE(BadDefaultValueType, "<?php class C { function f(int $i1 = array()) {} }");
  VE(BadDefaultValueType, "<?php function f(int $i1 = array()) {}");
  return true;
}

bool TestCodeError::TestInvalidMethodDefinition() {
  VE(InvalidMethodDefinition, "<?php interface I {public function f() {}}");
  return true;
}
