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

#include "hphp/test/ext/test_parser_expr.h"
#include "hphp/compiler/option.h"
#include "hphp/parser/scanner.h"

///////////////////////////////////////////////////////////////////////////////

bool TestParserExpr::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestExpressionList);
  RUN_TEST(TestAssignmentExpression);
  RUN_TEST(TestSimpleVariable);
  RUN_TEST(TestDynamicVariable);
  RUN_TEST(TestStaticMemberExpression);
  RUN_TEST(TestArrayElementExpression);
  RUN_TEST(TestStringOffsetExpression);
  RUN_TEST(TestDynamicFunctionCall);
  RUN_TEST(TestSimpleFunctionCall);
  RUN_TEST(TestScalarExpression);
  RUN_TEST(TestObjectPropertyExpression);
  RUN_TEST(TestObjectMethodExpression);
  RUN_TEST(TestListAssignment);
  RUN_TEST(TestNewObjectExpression);
  RUN_TEST(TestUnaryOpExpression);
  RUN_TEST(TestBinaryOpExpression);
  RUN_TEST(TestQOpExpression);
  RUN_TEST(TestArrayPairExpression);
  RUN_TEST(TestClassConstantExpression);
  RUN_TEST(TestParameterExpression);
  RUN_TEST(TestModifierExpression);
  RUN_TEST(TestConstant);
  RUN_TEST(TestEncapsListExpression);
  RUN_TEST(TestClosure);
  RUN_TEST(TestAwaitExpression);

  RUN_TEST(TestXHP);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestParserExpr::TestExpressionList() {
  // TestUnsetStatement
  // TestEchoStatement
  // TestForStatement
  // TestObjectPropertyExpression
  // TestListAssignment
  // TestUnaryOpExpression - internal_functions - isset_variables
  return true;
}

bool TestParserExpr::TestAssignmentExpression() {
  V("<?php $a = 1;",               "$a = 1;\n");
  V("<?php $a = &$b;",             "$a = &$b;\n");
  V("<?php $a = &new Test();",     "$a = &new Test();\n");
  V("<?php $a = &new $b();",       "$a = &new $b();\n");
  V("<?php $a = &new $$b();",      "$a = &new ${$b}();\n");
  V("<?php $a = &new Test::$b();", "$a = &new Test::$b();\n");
  V("<?php $a = &new $b->c();",    "$a = &new $b->c();\n");
  V("<?php $a = &new $b->c->d();", "$a = &new $b->c->d();\n");
  return true;
}

bool TestParserExpr::TestSimpleVariable() {
  V("<?php $a = $a;", "$a = $a;\n");
  return true;
}

bool TestParserExpr::TestDynamicVariable() {
  V("<?php $a = ${$a + $b};", "$a = ${$a + $b};\n");
  V("<?php $a = $$a;",        "$a = ${$a};\n");
  V("<?php $a = ${$a};",      "$a = ${$a};\n");
  V("<?php $a = $$$a;",       "$a = ${${$a}};\n");
  return true;
}

bool TestParserExpr::TestStaticMemberExpression() {
  V("<?php $a = Test::$a;", "$a = Test::$a;\n");
  return true;
}

bool TestParserExpr::TestArrayElementExpression() {
  V("<?php $a = $b[$a + $b];", "$a = $b[$a + $b];\n");
  V("<?php $a = $b[];",        "$a = $b[];\n");
  return true;
}

bool TestParserExpr::TestStringOffsetExpression() {
  V("<?php $a = $b{$a + $b};", "$a = $b[$a + $b];\n");
  return true;
}

bool TestParserExpr::TestDynamicFunctionCall() {
  V("<?php $test();",       "$test();\n");
  V("<?php Test::$test();", "Test::$test();\n");
  return true;
}

bool TestParserExpr::TestSimpleFunctionCall() {
  V("<?php test();",       "test();\n");
  V("<?php Test::test();", "Test::test();\n");
  V("<?php test(&$a);",    "test(&$a);\n");
  return true;
}

bool TestParserExpr::TestScalarExpression() {
  V("<?php A;",            "A;\n");            // T_STRING
  V("<?php \"$a[0xFF]\";", "$a[0xFF];\n");     // T_NUM_STRING
  V("<?php 12;",           "12;\n");           // T_LNUMBER
  V("<?php 0xFF;",         "0xFF;\n");         // T_LNUMBER
  V("<?php 1.2;",          "1.2;\n");          // T_DNUMBER
  V("<?php 'A';",          "'A';\n");          // T_CONSTANT_ENCAPSED_STRING
  V("<?php \"A\";",        "'A';\n");          // T_CONSTANT_ENCAPSED_STRING
  V("<?php __LINE__;",     "__LINE__;\n");     // T_LINE
  V("<?php __FILE__;",     "__FILE__;\n");     // T_FILE
  V("<?php __DIR__;",      "__DIR__;\n");     // T_DIR
  V("<?php __CLASS__;",    "__CLASS__;\n");    // T_CLASS_C
  V("<?php __METHOD__;",   "__METHOD__;\n");   // T_METHOD_C
  V("<?php __FUNCTION__;", "__FUNCTION__;\n"); // T_FUNC_C
  V("<?php \"${a}\";",     "$a;\n");           // T_STRING_VARNAME
  return true;
}

bool TestParserExpr::TestObjectPropertyExpression() {
  V("<?php print $b->c;",                 "print $b->c;\n");
  V("<?php print ${b}->c;",               "print ${b}->c;\n");
  V("<?php print ${$b}->c;",              "print ${$b}->c;\n");
  V("<?php print $b[]->c;",               "print $b[]->c;\n");
  V("<?php print $b[$a]->c;",             "print $b[$a]->c;\n");
  V("<?php print $b{$a}->c;",             "print $b[$a]->c;\n");
  V("<?php print $b{$a}[]->c;",           "print $b[$a][]->c;\n");
  V("<?php print $b{$a}[$c]->c;",         "print $b[$a][$c]->c;\n");
  V("<?php print $b{$a}[$c]{$d}->c;",     "print $b[$a][$c][$d]->c;\n");
  V("<?php print $b{$a}[$c]{$d}->c[];",   "print $b[$a][$c][$d]->c[];\n");
  V("<?php print $b{$a}[$c]{$d}->c[$e];", "print $b[$a][$c][$d]->c[$e];\n");
  V("<?php print $b{$a}[$c]{$d}->c{$e};", "print $b[$a][$c][$d]->c[$e];\n");

  V("<?php print $b{$a}[$c]{$d}->c{$e}->f;",
    "print $b[$a][$c][$d]->c[$e]->f;\n");

  V("<?php print $b{$a}[$c]{$d}->c{$e}->f[];",
    "print $b[$a][$c][$d]->c[$e]->f[];\n");

  return true;
}

bool TestParserExpr::TestObjectMethodExpression() {
  V("<?php echo $b->c();",                 "echo $b->c();\n");
  V("<?php echo ${b}->c();",               "echo ${b}->c();\n");
  V("<?php echo ${$b}->c();",              "echo ${$b}->c();\n");
  V("<?php echo $b[]->c();",               "echo $b[]->c();\n");
  V("<?php echo $b[$a]->c();",             "echo $b[$a]->c();\n");
  V("<?php echo $b{$a}->c();",             "echo $b[$a]->c();\n");
  V("<?php echo $b{$a}[]->c();",           "echo $b[$a][]->c();\n");
  V("<?php echo $b{$a}[$c]->c();",         "echo $b[$a][$c]->c();\n");
  V("<?php echo $b{$a}[$c]{$d}->c();",     "echo $b[$a][$c][$d]->c();\n");
  V("<?php echo $b{$a}[$c]{$d}->c[]();",   "echo $b[$a][$c][$d]->c[]();\n");
  V("<?php echo $b{$a}[$c]{$d}->c[$e]();", "echo $b[$a][$c][$d]->c[$e]();\n");
  V("<?php echo $b{$a}[$c]{$d}->c{$e}();", "echo $b[$a][$c][$d]->c[$e]();\n");

  V("<?php echo $b{$a}[$c]{$d}->c{$e}->f();",
    "echo $b[$a][$c][$d]->c[$e]->f();\n");

  V("<?php echo $b{$a}[$c]{$d}->c{$e}->f[]();",
    "echo $b[$a][$c][$d]->c[$e]->f[]();\n");

  V("<?php $b{$a}[$c]{$d}($p1,$p2)->c{$e}($p3,$p4)->f[]($p5,$p6);",
    "$b[$a][$c][$d]($p1, $p2)->c[$e]($p3, $p4)->f[]($p5, $p6);\n");

  return true;
}

bool TestParserExpr::TestListAssignment() {
  V("<?php list() = 1;",               "list() = 1;\n");
  V("<?php list(,) = 1;",              "list(, ) = 1;\n");
  V("<?php list($a,) = 1;",            "list($a, ) = 1;\n");
  V("<?php list(,$b) = 1;",            "list(, $b) = 1;\n");
  V("<?php list($b) = 1;",             "list($b) = 1;\n");
  V("<?php list($a,$b) = 1;",          "list($a, $b) = 1;\n");
  V("<?php list($a,list($c),$b) = 1;", "list($a, list($c), $b) = 1;\n");
  V("<?php list($a,list(),$b) = 1;",   "list($a, list(), $b) = 1;\n");
  return true;
}

bool TestParserExpr::TestNewObjectExpression() {
  V("<?php new Test;",                 "new Test();\n");
  V("<?php new $b();",                 "new $b();\n");
  V("<?php new $b;",                   "new $b();\n");

  return true;
}

bool TestParserExpr::TestUnaryOpExpression() {
  V("<?php clone $a;",        "clone $a;\n");
  V("<?php ++$a;",            "++$a;\n");
  V("<?php --$a;",            "--$a;\n");
  V("<?php $a++;",            "$a++;\n");
  V("<?php $a--;",            "$a--;\n");
  V("<?php +$a;",             "+$a;\n");
  V("<?php -$a;",             "-$a;\n");
  V("<?php !$a;",             "!$a;\n");
  V("<?php ~$a;",             "~$a;\n");
  V("<?php ($a);",            "$a;\n");
  V("<?php (int)$a;",         "(int)$a;\n");
  V("<?php (real)$a;",        "(double)$a;\n");
  V("<?php (string)$a;",      "(string)$a;\n");
  V("<?php (array)$a;",       "(array)$a;\n");
  V("<?php (object)$a;",      "(object)$a;\n");
  V("<?php (bool)$a;",        "(bool)$a;\n");
  V("<?php (unset)$a;",       "(unset)$a;\n");
  V("<?php exit;",            "exit();\n");
  V("<?php exit();",          "exit();\n");
  V("<?php exit($a);",        "exit($a);\n");
  V("<?php @$a;",             "@$a;\n");
  V("<?php array($a);",       "array($a);\n");
  V("<?php print $a;",        "print $a;\n");
  V("<?php isset($a);",       "isset($a);\n");
  V("<?php empty($a);",       "empty($a);\n");
  V("<?php include $a;",      "include $a;\n");
  V("<?php include_once 1;",  "include_once 1;\n");
  V("<?php eval($a);",        "eval($a);\n");
  V("<?php require $a;",      "require $a;\n");
  V("<?php require_once 1;",  "require_once 1;\n");
  return true;
}

bool TestParserExpr::TestBinaryOpExpression() {
  V("<?php $a += A;",         "$a += A;\n");
  V("<?php $a -= A;",         "$a -= A;\n");
  V("<?php $a *= A;",         "$a *= A;\n");
  V("<?php $a /= A;",         "$a /= A;\n");
  V("<?php $a .= A;",         "$a .= A;\n");
  V("<?php $a %= A;",         "$a %= A;\n");
  V("<?php $a &= A;",         "$a &= A;\n");
  V("<?php $a |= A;",         "$a |= A;\n");
  V("<?php $a ^= A;",         "$a ^= A;\n");
  V("<?php $a <<= A;",        "$a <<= A;\n");
  V("<?php $a >>= A;",        "$a >>= A;\n");
  V("<?php $a || A;",         "$a || A;\n");
  V("<?php $a && A;",         "$a && A;\n");
  V("<?php $a or A;",         "$a or A;\n");
  V("<?php $a and A;",        "$a and A;\n");
  V("<?php $a xor A;",        "$a xor A;\n");
  V("<?php $a | A;",          "$a | A;\n");
  V("<?php $a & A;",          "$a & A;\n");
  V("<?php $a ^ A;",          "$a ^ A;\n");
  V("<?php $a . A;",          "$a . A;\n");
  V("<?php $a + A;",          "$a + A;\n");
  V("<?php $a - A;",          "$a - A;\n");
  V("<?php $a * A;",          "$a * A;\n");
  V("<?php $a / A;",          "$a / A;\n");
  V("<?php $a % A;",          "$a % A;\n");
  V("<?php $a << A;",         "$a << A;\n");
  V("<?php $a >> A;",         "$a >> A;\n");
  V("<?php $a === A;",        "$a === A;\n");
  V("<?php $a !== A;",        "$a !== A;\n");
  V("<?php $a == A;",         "$a == A;\n");
  V("<?php $a != A;",         "$a != A;\n");
  V("<?php $a < A;",          "$a < A;\n");
  V("<?php $a <= A;",         "$a <= A;\n");
  V("<?php $a > A;",          "$a > A;\n");
  V("<?php $a >= A;",         "$a >= A;\n");
  V("<?php $a instanceof A;", "$a instanceof A;\n");
  return true;
}

bool TestParserExpr::TestQOpExpression() {
  V("<?php $a ? 2 : 3;", "$a ? 2 : 3;\n");
  return true;
}

bool TestParserExpr::TestArrayPairExpression() {
  V("<?php array();",                     "array();\n");
  V("<?php array($a);",                   "array($a);\n");
  V("<?php array($a, $b);",               "array($a, $b);\n");
  V("<?php array($a, $b,);",              "array($a, $b);\n");
  V("<?php array($a => $b);",             "array($a => $b);\n");
  V("<?php array($a => $b, $c => $d);",   "array($a => $b, $c => $d);\n");
  V("<?php array($a => $b, $c => $d,);",  "array($a => $b, $c => $d);\n");

  V("<?php array(&$a);",                  "array(&$a);\n");
  V("<?php array(&$a, &$b);",             "array(&$a, &$b);\n");
  V("<?php array($a => &$b);",            "array($a => &$b);\n");
  V("<?php array($a => &$b, $c => &$d);", "array($a => &$b, $c => &$d);\n");

  V("<?php function a() { static $a = array();}",
    "function a() {\nstatic $a = array();\n}\n");
  V("<?php function a() { static $a = array(a);}",
    "function a() {\nstatic $a = array(a);\n}\n");
  V("<?php function a() { static $a = array(a, b);}",
    "function a() {\nstatic $a = array(a, b);\n}\n");
  V("<?php function a() { static $a = array(a, b,);}",
    "function a() {\nstatic $a = array(a, b);\n}\n");
  V("<?php function a() { static $a = array(a => b);}",
    "function a() {\nstatic $a = array(a => b);\n}\n");
  V("<?php function a() { static $a = array(a => b, c => d);}",
    "function a() {\nstatic $a = array(a => b, c => d);\n}\n");
  V("<?php function a() { static $a = array(a => b, c => d,);}",
    "function a() {\nstatic $a = array(a => b, c => d);\n}\n");
  return true;
}

bool TestParserExpr::TestClassConstantExpression() {
  V("<?php function a() { static $a = A::b;}",
    "function a() {\nstatic $a = A::b;\n}\n");
  return true;
}

bool TestParserExpr::TestParameterExpression() {
  V("<?php function a($a=1,$b) {}",  "function a($a = 1, $b) {\n}\n");

  V("<?php function a() {}",         "function a() {\n}\n");
  V("<?php function a($a) {}",       "function a($a) {\n}\n");
  V("<?php function a($a,$b) {}",    "function a($a, $b) {\n}\n");
  V("<?php function a(&$a) {}",      "function a(&$a) {\n}\n");
  V("<?php function a(&$a,$b) {}",   "function a(&$a, $b) {\n}\n");
  V("<?php function a($a,&$b) {}",   "function a($a, &$b) {\n}\n");
  V("<?php function a(TT $a) {}",    "function a(TT $a) {\n}\n");
  V("<?php function a(array $a) {}", "function a(array $a) {\n}\n");
  V("<?php function a($a=1) {}",     "function a($a = 1) {\n}\n");
  V("<?php function a($a,$b=1) {}",  "function a($a, $b = 1) {\n}\n");
  return true;
}

bool TestParserExpr::TestModifierExpression() {
  V("<?php class a { public $a;}",    "class a {\npublic $a;\n}\n");
  V("<?php class a { protected $a;}", "class a {\nprotected $a;\n}\n");
  V("<?php class a { private $a;}",   "class a {\nprivate $a;\n}\n");
  V("<?php class a { static $a;}",
    "class a {\npublic static $a;\n}\n");
  V("<?php class a { public static $a;}",
    "class a {\npublic static $a;\n}\n");

  return true;
}

bool TestParserExpr::TestConstant() {
  V("<?php class a { const A = 1;}",  "class a {\nconst A = 1;\n}\n");
  V("<?php class a { const A=1,B=2;}","class a {\nconst A = 1, B = 2;\n}\n");
  return true;
}

bool TestParserExpr::TestEncapsListExpression() {
  V("<?php '\\'\\\\\\\"';",       "\"'\\\\\\\\\".'\"';\n");
  V("<?php '$a$b';",              "'$a$b';\n");
  V("<?php \"$a$b\";",            "$a . $b;\n");
  V("<?php <<<EOM\n$a$b\nEOM;\n", "$a . $b . '';\n");
  V("<?php `$a$b`;",              "shell_exec($a . $b);\n");
  V("<?php \"[\\b$/\";",          "'['.\"\\\\\".'b$/';\n");
  V("<?php \"]\\b$/\";",          "']'.\"\\\\\".'b$/';\n");
  V("<?php \"{\\b$/\";",          "'{'.\"\\\\\".'b$/';\n");
  V("<?php \"}\\b$/\";",          "'}'.\"\\\\\".'b$/';\n");
  V("<?php \"->\\b$/\";",         "'->'.\"\\\\\".'b$/';\n");
  V("<?php \"$a[b]\";",           "$a['b'];\n");
  V("<?php \"\\\"\";",            "'\"';\n");
  V("<?php \"\\n\";",             "\"\\n\";\n");
  V("<?php \"\\n$a\";",           "\"\\n\" . $a;\n");
  V("<?php \"\\\"$a\";",          "'\"' . $a;\n");
  V("<?php \"\\$a\";",            "'$a';\n");
  V("<?php \"${a}\";",            "$a;\n");
  return true;
}

bool TestParserExpr::TestClosure() {
  V("<?php $a = function ($a) { return $a;};",
    "$a = function ($a) {\nreturn $a;\n}\n;\n");

  V("<?php $a = function ($a) use ($var) { return $var + $a;};",
    "$a = function ($a) use ($var) {\nreturn $var + $a;\n}\n;\n");

  return true;
}

bool TestParserExpr::TestAwaitExpression() {
  V("<?hh async function foo() { await goo(); await $hoo; return; }",
    "async function foo() {\n"
    "await goo();\n"
    "await $hoo;\n"
    "return;\n"
    "}\n");

  V("<?hh async function foo() { $a = await b(); return; }",
    "async function foo() {\n"
    "$a = await b();\n"
    "return;\n"
    "}\n");

  V("<?hh async function foo() { return await $a; }",
    "async function foo() {\n"
    "return await $a;\n"
    "}\n");

  V("<?hh $a = async function($a) { return await $a; }; ",
    "$a = async function ($a) {\nreturn await $a;\n}\n;\n");

  V("<?hh class A { "
    "static async function foo() { return 1; } "
    "private async function goo() { return 2; } }",
    "class A {\n"
    "public static async function foo() {\nreturn 1;\n}\n"
    "private async function goo() {\nreturn 2;\n}\n}\n");

  return true;
}

bool TestParserExpr::TestXHP() {
  // basics
  V("<?hh $x = <thing />;",
    "$x = new xhp_thing(array(), array(), __FILE__, __LINE__);\n");

  // white spaces
  V("<?hh $x = <x> a{ 'b' }c </x>;",
    "$x = new xhp_x(array(), array(' a', 'b', 'c '), __FILE__, __LINE__);\n");
  V("<?hh $x = <x> a { 'b' } c </x>;",
    "$x = new xhp_x(array(), array(' a ', 'b', ' c '), __FILE__, __LINE__);\n");
  V("<?hh $x = <x>\n    foo\n   </x>;",
    "$x = new xhp_x(array(), array(' foo '), __FILE__, __LINE__);\n");
  V("<?hh $x = <x>\n    foo\n   bar\n   </x>;",
    "$x = new xhp_x(array(), array(' foo bar '), __FILE__, __LINE__);\n");

  // attributes
  V("<?hh $x = <x:y attr={:tag::CONSTANT} />;",
    "$x = new xhp_x__y(array('attr' => xhp_tag::CONSTANT), array(), __FILE__, __LINE__);\n");
  V("<?hh $x = <a b=\"&nbsp;\">c</a>;",
    "$x = new xhp_a(array('b' => '\xC2\xA0'), array('c'), __FILE__, __LINE__);\n");
  V("<?hh $x = <a b=\"\" />;",
    "$x = new xhp_a(array('b' => ''), array(), __FILE__, __LINE__);\n");

  // children
  V("<?hh $x = <x> <x /> {'a'} </x>;",
    "$x = new xhp_x(array(), array(new xhp_x(array(), array(), __FILE__, __LINE__), 'a'), __FILE__, __LINE__);\n");
  V("<?hh $x = <x> {'a'}<x /></x>;",
    "$x = new xhp_x(array(), array('a', new xhp_x(array(), array(), __FILE__, __LINE__)), __FILE__, __LINE__);");
  V("<?hh $x = <x>\n<x>\n</x>.\n</x>;",
    "$x = new xhp_x(array(), array(new xhp_x(array(), array(), __FILE__, __LINE__), '. '), __FILE__, __LINE__);\n");
  V("<?hh <div><a />=<a /></div>;",
    "new xhp_div(array(), array(new xhp_a(array(), array(), __FILE__, __LINE__), '=', "
    "new xhp_a(array(), array(), __FILE__, __LINE__)), __FILE__, __LINE__);\n");

  // closing tag
  V("<?hh $x = <a><a><a>hi</a></></a>;",
    "$x = new xhp_a(array(), array(new xhp_a(array(), "
    "array(new xhp_a(array(), array('hi'), __FILE__, __LINE__)), __FILE__, __LINE__)), __FILE__, __LINE__);\n");

  // class name with PHP keyword
  V("<?hh class :a:b:switch-links { }",
    "class xhp_a__b__switch_links {\n}\n");

  V("<?hh if ($obj instanceof :a:b:switch-links) { }",
    "if ($obj instanceof xhp_a__b__switch_links) {\n}\n");

  // class attributes
  V("<?hh class :thing { attribute Thing a, Thing b; }",

    "class xhp_thing {\n"
    "protected static function &__xhpAttributeDeclaration() {\n"
    "static $_ = -1;\n"
    "if ($_ === -1) {\n"
    "$_ = array_merge(parent::__xhpAttributeDeclaration(), "
    "array('a' => array(5, 'Thing', null, 0), "
    "'b' => array(5, 'Thing', null, 0)));\n"
    "}\n"
    "return $_;\n"
    "}\n"
    "}\n");

  // enum attributes
  V("<?hh class :thing { attribute enum { 123, 456 } a; }",

    "class xhp_thing {\n"
    "protected static function &__xhpAttributeDeclaration() {\n"
    "static $_ = -1;\n"
    "if ($_ === -1) {\n"
    "$_ = array_merge(parent::__xhpAttributeDeclaration(), "
    "array('a' => array(7, array(123, 456), null, 0)));\n"
    "}\n"
    "return $_;\n"
    "}\n"
    "}\n");

  // base attributes
  V("<?hh class :foo { attribute string foo; }"
    " class :bar { attribute :foo, :foo, string bar; }",

    "class xhp_foo {\n"
    "protected static function &__xhpAttributeDeclaration() {\n"
    "static $_ = -1;\n"
    "if ($_ === -1) {\n"
    "$_ = array_merge(parent::__xhpAttributeDeclaration(), "
    "array('foo' => array(1, null, null, 0)));\n"
    "}\n"
    "return $_;\n"
    "}\n"
    "}\n"
    "class xhp_bar {\n"
    "protected static function &__xhpAttributeDeclaration() {\n"
    "static $_ = -1;\n"
    "if ($_ === -1) {\n"
    "$_ = array_merge(parent::__xhpAttributeDeclaration(), "
    "xhp_foo::__xhpAttributeDeclaration(), "
    "xhp_foo::__xhpAttributeDeclaration(), "
    "array('bar' => array(1, null, null, 0)));\n"
    "}\n"
    "return $_;\n"
    "}\n"
    "}\n");

  // attribute default and required
  V("<?hh class :thing { attribute int a = 123 @required, var b; }",

    "class xhp_thing {\n"
    "protected static function &__xhpAttributeDeclaration() {\n"
    "static $_ = -1;\n"
    "if ($_ === -1) {\n"
    "$_ = array_merge(parent::__xhpAttributeDeclaration(), "
    "array('a' => array(3, null, 123, 1), 'b' => array(6, null, null, 0)));\n"
    "}\n"
    "return $_;\n"
    "}\n"
    "}\n");

  // categories
  V("<?hh class :thing { category %a:foo, %b; }",

    "class xhp_thing {\n"
    "protected function &__xhpCategoryDeclaration() {\n"
    "static $_ = array('a:foo' => 1, 'b' => 1);\n"
    "return $_;\n"
    "}\n"
    "}\n");

  // children
  V("<?hh class :thing { children(any,any); }",

    "class xhp_thing {\n"
    "protected function &__xhpChildrenDeclaration() {\n"
    "static $_ = array(0, 5, array(4, array(0, 1, null), "
    "array(0, 1, null)));\n"
    "return $_;\n"
    "}\n"
    "}\n");

  V("<?hh class :thing { children any; }",

    "class xhp_thing {\n"
    "protected function &__xhpChildrenDeclaration() {\n"
    "static $_ = 1;\n"
    "return $_;\n"
    "}\n"
    "}\n");

  V("<?hh class :thing { children ((:a:foo | %b:bar)+, pcdata); }",

    "class xhp_thing {\n"
    "protected function &__xhpChildrenDeclaration() {\n"
    "static $_ = array(0, 5, array(4, array(3, 5, "
    "array(5, array(0, 3, 'xhp_a__foo'), array(0, 4, 'b__bar'))), "
    "array(0, 2, null)));\n"
    "return $_;\n"
    "}\n"
    "}\n");

  // comments
  V("<?hh class :thing {\n"
    "  category %a:foo, %b; // comments\n"
    "  children any; }",

    "class xhp_thing {\n"
    "protected function &__xhpCategoryDeclaration() {\n"
    "static $_ = array('a:foo' => 1, 'b' => 1);\n"
    "return $_;\n"
    "}\n"
    "protected function &__xhpChildrenDeclaration() {\n"
    "static $_ = 1;\n"
    "return $_;\n"
    "}\n"
    "}\n");

  // multiple interleaved
  V("<?hh "
    "class :thing { "
    "  attribute Thing a; category %a; children any; "
    "  function foo() {}"
    "  attribute Thing b;"
    "  function bar() {}"
    "}",

    "class xhp_thing {\n"
    "protected function &__xhpCategoryDeclaration() {\n"
    "static $_ = array('a' => 1);\n"
    "return $_;\n"
    "}\n"
    "protected function &__xhpChildrenDeclaration() {\n"
    "static $_ = 1;\n"
    "return $_;\n"
    "}\n"
    "public function foo() {\n"
    "}\n"
    "public function bar() {\n"
    "}\n"
    "protected static function &__xhpAttributeDeclaration() {\n"
    "static $_ = -1;\n"
    "if ($_ === -1) {\n"
    "$_ = array_merge(parent::__xhpAttributeDeclaration(), "
    "array('a' => array(5, 'Thing', null, 0), "
    "'b' => array(5, 'Thing', null, 0)));\n"
    "}\n"
    "return $_;\n"
    "}\n"
    "}\n");

  return true;
}
