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

#include <test/test_trans_expr.h>

///////////////////////////////////////////////////////////////////////////////

TestTransformerExpr::TestTransformerExpr() {
  m_verbose = false;
}

bool TestTransformerExpr::RunTests(const std::string &which) {
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
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestTransformerExpr::TestExpressionList() {
  // TestUnsetStatement
  // TestEchoStatement
  // TestForStatement
  // TestObjectPropertyExpression
  // TestListAssignment
  // TestUnaryOpExpression - internal_functions - isset_variables
  return true;
}

bool TestTransformerExpr::TestAssignmentExpression() {
  VT("<?php $a = 1;",
     "Variant gv_a;\n"
     "\n"
     "gv_a = 1;\n");
  VT("<?php $a = &$b;",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "gv_a = ref(gv_b);\n");
  VT("<?php class Test {} $a = &new Test();",
     "Variant gv_a;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "gv_a = sp_test(sp_test(NEW(c_test)())->create());\n");
  VT("<?php $a = &new $b();",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "gv_a = create_object(toString(gv_b), Array());\n");
  VT("<?php $a = &new $$b();",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "gv_a = create_object((toString(variables->get(toString(gv_b)))), Array());\n");
  VT("<?php class Test { static $b;} $a = &new Test::$b();",
     "Variant gv_a;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: static Variant s_b;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "Variant c_test::s_b;\n"
     "gv_a = create_object((toString(c_test::s_b)), Array());\n");
  VT("<?php $a = &new $b->c();",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "gv_a = create_object((toString(toObject(gv_b).o_get(\"c\", 0x000000000002B608LL))), Array());\n");
  VT("<?php $a = &new $b->c->d();",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "gv_a = create_object((toString(toObject(toObject(gv_b).o_get(\"c\", 0x000000000002B608LL)).o_get(\"d\", 0x000000000002B609LL))), Array());\n");
  return true;
}

bool TestTransformerExpr::TestSimpleVariable() {
  VT("<?php $a = $a;", "Variant gv_a;\n"
     "\n"
     "gv_a = gv_a;\n");
  return true;
}

bool TestTransformerExpr::TestDynamicVariable() {
  VT("<?php $a = $$a;",
     "Variant gv_a;\n"
     "\n"
     "gv_a = variables->get(toString(gv_a));\n");
  return true;

  VT("<?php $a = ${$a + $b};",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "gv_a = variables->get(toString(gv_a + gv_b));\n");
  VT("<?php $a = $$a;",
     "Variant gv_a;\n"
     "\n"
     "gv_a = variables->get(toString(gv_a));\n");
  VT("<?php $a = ${$a};",
     "Variant gv_a;\n"
     "\n"
     "gv_a = variables->get(toString(gv_a));\n");
  VT("<?php $a = $$$a;",
     "Variant gv_a;\n"
     "\n"
     "gv_a = variables->get(toString(variables->get(toString(gv_a))));\n");
  return true;
}

bool TestTransformerExpr::TestStaticMemberExpression() {
  VT("<?php $a = Test::a;",
     "Variant gv_a;\n"
     "\n"
     "gv_a = throw_fatal(\"unknown class constant test::a\");\n");
  VT("<?php class Test { static $a; } $a = Test::a;",
     "Variant gv_a;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: static Variant s_a;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "Variant c_test::s_a;\n"
     "gv_a = throw_fatal(\"unknown class constant test::a\");\n");
  VT("<?php class Test { static $a; } $a = Test::$a;",
     "Variant gv_a;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: static Variant s_a;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "Variant c_test::s_a;\n"
     "gv_a = c_test::s_a;\n");
  return true;
}

bool TestTransformerExpr::TestArrayElementExpression() {
  VT("<?php $a = $b[$a + $b];",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "gv_a = ((Variant)gv_b.rvalAt(gv_a + gv_b));\n");
  VT("<?php $a = $b[];",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "gv_a = ((Variant)toArray(gv_b).lvalAt());\n");
  return true;
}

bool TestTransformerExpr::TestStringOffsetExpression() {
  VT("<?php $a = $b{$a + $b};",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "gv_a = ((Variant)gv_b.rvalAt(gv_a + gv_b));\n");
  return true;
}

bool TestTransformerExpr::TestDynamicFunctionCall() {
  VT("<?php $test();",
     "Variant gv_test;\n"
     "\n"
     "invoke(toString(gv_test), Array(), -1);\n");
  VT("<?php class Test {} Test::$test();",
     "Variant gv_test;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "c_test::os_invoke(toString(gv_test), Array(), -1);\n");
  VT("<?php Test::$test();",
     "Variant gv_test;\n"
     "\n"
     "throw_fatal(\"unknown class test\");\n");
  return true;
}

bool TestTransformerExpr::TestSimpleFunctionCall() {
  VT("<?php function test() {} test();",
     "void f_test();\n"
     "void f_test() {\n"
     "}\n"
     "f_test();\n");
  VT("<?php function test() {} print test();",
     "void f_test();\n"
     "void f_test() {\n"
     "}\n"
     "print((f_test(), null));\n");
  VT("<?php test(&$a);",
     "Variant gv_a;\n"
     "\n"
     "invokeFailed(\"test\", Array(NEW(ArrayElement)(ref(gv_a)), NULL), 0x000000017C9E6865LL);\n");
  VT("<?php test();",
     "invokeFailed(\"test\", Array(), 0x000000017C9E6865LL);\n");

  VT("<?php class Test { public static function t() {}} Test::t();",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: static void t_t();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void c_test::t_t() {\n"
     "}\n"
     "c_test::t_t();\n");
  VT("<?php Test::test();",
     "throw_fatal(\"unknown class test\", ((void*)NULL));\n");

  VT("<?php function test() { func_get_arg();} test();",
     "void f_test(int num_args, Array args = Array());\n"
     "void f_test(int num_args, Array args /* = Array() */) {\n"
     "  func_get_arg(num_args, Array(),args);\n"
     "}\n"
     "f_test(0);\n");
  VT("<?php function test() { func_get_arg();} test($a);",
     "Variant gv_a;\n"
     "\n"
     "void f_test(int num_args, Array args = Array());\n"
     "void f_test(int num_args, Array args /* = Array() */) {\n"
     "  func_get_arg(num_args, Array(),args);\n"
     "}\n"
     "f_test(1, Array(NEW(ArrayElement)(gv_a), NULL));\n");
  VT("<?php function test() { func_get_arg();} test($a, $b);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "void f_test(int num_args, Array args = Array());\n"
     "void f_test(int num_args, Array args /* = Array() */) {\n"
     "  func_get_arg(num_args, Array(),args);\n"
     "}\n"
     "f_test(2, Array(NEW(ArrayElement)(gv_a), NEW(ArrayElement)(gv_b), NULL));\n");
  VT("<?php function test($a) { func_get_arg();} test($a, $b);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "void f_test(int num_args, CVarRef v_a, Array args = Array());\n"
     "void f_test(int num_args, CVarRef v_a, Array args /* = Array() */) {\n"
     "  func_get_arg(num_args, Array(NEW(ArrayElement)(v_a), NULL),args);\n"
     "}\n"
     "f_test(2, gv_a, Array(NEW(ArrayElement)(gv_b), NULL));\n");
  return true;
}

bool TestTransformerExpr::TestScalarExpression() {
  VT("<?php 12;",           "12;\n");           // T_LNUMBER
  VT("<?php 0xFF;",         "0xFF;\n");         // T_LNUMBER
  VT("<?php 1.2;",          "1.2;\n");          // T_DNUMBER
  VT("<?php \"A\";",        "\"A\";\n");        // T_CONSTANT_ENCAPSED_STRING
  VT("<?php 'A';",          "\"A\";\n");        // T_CONSTANT_ENCAPSED_STRING
  VT("<?php '\"';",         "\"\\\"\";\n");     // T_CONSTANT_ENCAPSED_STRING
  VT("<?php \"$a[0xFF]\";",                     // T_NUM_STRING
     "Variant gv_a;\n"
     "\n"
     "toString(((Variant)gv_a.rvalAt(0xFF, 0x77CFA1EEF01BCA90LL)));\n");
  VT("<?php A;",                                // T_STRING
     "const String k_A = \"A\";\n"
     "\n"
     "k_A;\n");
  VT("<?php \"${a}\";",                         // T_STRING_VARNAME
     "Variant gv_a;\n"
     "\n"
     "toString(gv_a);\n");
  return true;
}

bool TestTransformerExpr::TestObjectPropertyExpression() {
  VT("<?php class Test { public $c; } $b = new Test(); print $b->c;",
     "Variant gv_b;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_c;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "gv_b = sp_test(sp_test(NEW(c_test)())->create());\n"
     "print(toString(toObject(gv_b).o_get(\"c\", 0x000000000002B608LL)));\n"
     );

  VT("<?php print $b->c;",
     "Variant gv_b;\n"
     "\n"
     "print(toString(toObject(gv_b).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print ${b}->c;",
     "const String k_b = \"b\";\n"
     "\n"
     "print(toString(toObject(variables->get(k_b)).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print ${$b}->c;",
     "Variant gv_b;\n"
     "\n"
     "print(toString(toObject(variables->get(toString(gv_b))).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print $b[]->c;",
     "Variant gv_b;\n"
     "\n"
     "print(toString(toObject(((Variant)toArray(gv_b).lvalAt())).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print $b[$a]->c;",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "print(toString(toObject(((Variant)gv_b.rvalAt(gv_a))).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print $b{$a}->c;",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "print(toString(toObject(((Variant)gv_b.rvalAt(gv_a))).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print $b{$a}[]->c;",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "print(toString(toObject(((Variant)toArray(((Variant)gv_b.rvalAt(gv_a))).lvalAt())).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print $b{$a}[$c]->c;",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "Variant gv_c;\n"
     "\n"
     "print(toString(toObject(((Variant)((Variant)gv_b.rvalAt(gv_a)).rvalAt(gv_c))).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print $b{$a}[$c]{$d}->c;",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "Variant gv_c;\n"
     "Variant gv_d;\n"
     "\n"
     "print(toString(toObject(((Variant)((Variant)((Variant)gv_b.rvalAt(gv_a)).rvalAt(gv_c)).rvalAt(gv_d))).o_get(\"c\", 0x000000000002B608LL)));\n");
  VT("<?php print $b->c[];",
     "Variant gv_b;\n"
     "\n"
     "print(toString(((Variant)toArray(toObject(gv_b).o_get(\"c\", 0x000000000002B608LL)).lvalAt())));\n");
  VT("<?php print $b{$a}[$c]{$d}->c[];",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "Variant gv_c;\n"
     "Variant gv_d;\n"
     "\n"
     "print(toString(((Variant)toArray(toObject(((Variant)((Variant)((Variant)gv_b.rvalAt(gv_a)).rvalAt(gv_c)).rvalAt(gv_d))).o_get(\"c\", 0x000000000002B608LL)).lvalAt())));\n");

  return true;
}

bool TestTransformerExpr::TestObjectMethodExpression() {
  VT("<?php class Test { public $c; } $b = new Test(); echo $b->c();",
     "Variant gv_b;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_c;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "gv_b = sp_test(sp_test(NEW(c_test)())->create());\n"
     "echo(toString(toObject(gv_b)->o_invoke(\"c\", Array(), 0x000000000002B608LL)));\n"
     );

  VT("<?php class Test { public function c() {} } "
     "$b = new Test(); echo $b->c();",
     "Variant gv_b;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: void t_c();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void c_test::t_c() {\n"
     "}\n"
     "gv_b = sp_test(sp_test(NEW(c_test)())->create());\n"
     "echo((sp_test(gv_b)->t_c(), null));\n"
     );

  VT("<?php echo $b->c();",
     "Variant gv_b;\n"
     "\n"
     "echo(toString(toObject(gv_b)->o_invoke(\"c\", Array(), 0x000000000002B608LL)));\n");
  VT("<?php echo ${b}->c();",
     "const String k_b = \"b\";\n"
     "\n"
     "echo(toString(toObject(variables->get(k_b))->o_invoke(\"c\", Array(), 0x000000000002B608LL)));\n"
     );
  VT("<?php echo ${$b}->c();",
     "Variant gv_b;\n"
     "\n"
     "echo(toString(toObject(variables->get(toString(gv_b)))->o_invoke(\"c\", Array(), 0x000000000002B608LL)));\n");
  VT("<?php echo $b[]->c();",
     "Variant gv_b;\n"
     "\n"
     "echo(toString(toObject(((Variant)toArray(gv_b).lvalAt()))->o_invoke(\"c\", Array(), 0x000000000002B608LL)));\n");
  VT("<?php $b{$a}[$c]{$d}($p1,$p2)->c{$e}($p3,$p4)->f[]($p5,$p6);",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "Variant gv_c;\n"
     "Variant gv_d;\n"
     "Variant gv_p1;\n"
     "Variant gv_p2;\n"
     "Variant gv_e;\n"
     "Variant gv_p3;\n"
     "Variant gv_p4;\n"
     "Variant gv_p5;\n"
     "Variant gv_p6;\n"
     "\n"
     "invoke((toString(((Variant)toArray(toObject(invoke((toString(((Variant)toObject(invoke((toString(((Variant)((Variant)((Variant)gv_b.rvalAt(gv_a)).rvalAt(gv_c)).rvalAt(gv_d)))), Array(NEW(ArrayElement)(ref(gv_p1)), NEW(ArrayElement)(ref(gv_p2)), NULL), -1)).o_get(\"c\", 0x000000000002B608LL).rvalAt(gv_e)))), Array(NEW(ArrayElement)(ref(gv_p3)), NEW(ArrayElement)(ref(gv_p4)), NULL), -1)).o_get(\"f\", 0x000000000002B60BLL)).lvalAt()))), Array(NEW(ArrayElement)(ref(gv_p5)), NEW(ArrayElement)(ref(gv_p6)), NULL), -1);\n");

  return true;
}

bool TestTransformerExpr::TestListAssignment() {
  VT("<?php list() = 1;",
     "list_assign(toArray(1), (ListAssignmentElement *)NULL);\n");
  VT("<?php list(,) = 1;",
     "list_assign(toArray(1), (ListAssignmentElement *)NULL);\n");
  VT("<?php list($a,) = 1;",
     "Variant gv_a;\n"
     "\n"
     "list_assign(toArray(1), new ListAssignmentElement(gv_a, 0, -1), (ListAssignmentElement *)NULL);\n");
  VT("<?php list(,$b) = 1;",
     "Variant gv_b;\n"
     "\n"
     "list_assign(toArray(1), new ListAssignmentElement(gv_b, 1, -1), (ListAssignmentElement *)NULL);\n");
  VT("<?php list($b) = 1;",
     "Variant gv_b;\n"
     "\n"
     "list_assign(toArray(1), new ListAssignmentElement(gv_b, 0, -1), (ListAssignmentElement *)NULL);\n");
  VT("<?php list($a,$b) = 1;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "list_assign(toArray(1), new ListAssignmentElement(gv_a, 0, -1), new ListAssignmentElement(gv_b, 1, -1), (ListAssignmentElement *)NULL);\n");
  VT("<?php list($a,list($c),$b) = 1;",
     "Variant gv_a;\n"
     "Variant gv_c;\n"
     "Variant gv_b;\n"
     "\n"
     "list_assign(toArray(1), new ListAssignmentElement(gv_a, 0, -1), new ListAssignmentElement(gv_c, 1, 0, -1), new ListAssignmentElement(gv_b, 2, -1), (ListAssignmentElement *)NULL);\n");
  VT("<?php list($a,list(),$b) = 1;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "list_assign(toArray(1), new ListAssignmentElement(gv_a, 0, -1), new ListAssignmentElement(gv_b, 2, -1), (ListAssignmentElement *)NULL);\n");
  return true;
}

bool TestTransformerExpr::TestNewObjectExpression() {
  VT("<?php class Test {} new Test;",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "sp_test(sp_test(NEW(c_test)())->create());\n");

  VT("<?php new Test;",
     "throw_fatal_object(\"unknown class test\", ((void*)NULL));\n");
  VT("<?php new $b();",
     "Variant gv_b;\n"
     "\n"
     "create_object(toString(gv_b), Array());\n");
  VT("<?php new $b;",
     "Variant gv_b;\n"
     "\n"
     "create_object(toString(gv_b), Array());\n");
  VT("<?php class Test {} new Test($a);",
     "Variant gv_a;\n"
     "\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "throw_fatal_object(\"unknown class test\", ((void*)NULL));\n");
  VT("<?php new $b($a);",
     "Variant gv_b;\n"
     "Variant gv_a;\n"
     "\n"
     "create_object(toString(gv_b), Array(NEW(ArrayElement)(ref(gv_a)), NULL));\n");
  return true;
}

bool TestTransformerExpr::TestUnaryOpExpression() {
  VT("<?php clone $a;",        "Variant gv_a;\n"
     "\n"
     "f_clone(toObject(gv_a));\n");
  VT("<?php ++$a;",            "Variant gv_a;\n"
     "\n"
     "++gv_a;\n");
  VT("<?php --$a;",            "Variant gv_a;\n"
     "\n"
     "--gv_a;\n");
  VT("<?php $a++;",            "Variant gv_a;\n"
     "\n"
     "gv_a++;\n");
  VT("<?php $a--;",            "Variant gv_a;\n"
     "\n"
     "gv_a--;\n");
  VT("<?php +$a;",             "Variant gv_a;\n"
     "\n"
     "+gv_a;\n");
  VT("<?php -$a;",             "Variant gv_a;\n"
     "\n"
     "-gv_a;\n");
  VT("<?php !$a;",             "Variant gv_a;\n"
     "\n"
     "!(toBoolean(gv_a));\n");
  VT("<?php ~$a;",             "Variant gv_a;\n"
     "\n"
     "~gv_a;\n");
  VT("<?php ($a);",            "Variant gv_a;\n"
     "\n"
     "(gv_a);\n");
  VT("<?php (int)$a;",         "Variant gv_a;\n"
     "\n"
     "toInt64(gv_a);\n");
  VT("<?php (real)$a;",        "Variant gv_a;\n"
     "\n"
     "toDouble(gv_a);\n");
  VT("<?php (string)$a;",      "Variant gv_a;\n"
     "\n"
     "toString(gv_a);\n");
  VT("<?php (array)$a;",       "Variant gv_a;\n"
     "\n"
     "toArray(gv_a);\n");
  VT("<?php (object)$a;",      "Variant gv_a;\n"
     "\n"
     "toObject(gv_a);\n");
  VT("<?php (bool)$a;",        "Variant gv_a;\n"
     "\n"
     "toBoolean(gv_a);\n");
  VT("<?php (unset)$a;",       "Variant gv_a;\n"
     "\n"
     "unset(gv_a);\n");
  VT("<?php exit;",            "f_exit();\n");
  VT("<?php exit();",          "f_exit();\n");
  VT("<?php exit($a);",        "Variant gv_a;\n"
     "\n"
     "f_exit(gv_a);\n");
  VT("<?php @$a;",
     "Variant gv_a;\n"
     "\n"
     "(silenceInc(), silenceDec(gv_a));\n");
  VT("<?php array($a);",
     "Variant gv_a;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a), NULL);\n");
  VT("<?php print $a;",        "Variant gv_a;\n"
     "\n"
     "print(toString(gv_a));\n");
  VT("<?php isset($a);",       "Variant gv_a;\n"
     "\n"
     "isset(gv_a);\n");
  VT("<?php empty($a);",       "Variant gv_a;\n"
     "\n"
     "empty(gv_a);\n");
  VT("<?php include $a;",      "Variant gv_a;\n"
     "\n"
     "invokeFile(toString(gv_a), false, variables);\n");
  VT("<?php include_once 1;",  "invokeFile(toString(1), true, variables);\n");
  VT("<?php eval($a);",        "Variant gv_a;\n"
     "\n"
     "f_eval(toString(gv_a));\n");
  VT("<?php require $a;",      "Variant gv_a;\n"
     "\n"
     "invokeFile(toString(gv_a), false, variables);\n");
  VT("<?php require_once 1;",  "invokeFile(toString(1), true, variables);\n");
  return true;
}

bool TestTransformerExpr::TestBinaryOpExpression() {
  VT("<?php $a += $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a += gv_b;\n");
  VT("<?php $a -= $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a -= gv_b;\n");
  VT("<?php $a *= $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a *= gv_b;\n");
  VT("<?php $a /= $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a /= gv_b;\n");
  VT("<?php $a .= $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "concat_assign(gv_a, toString(gv_b));\n");
  VT("<?php $a %= $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a %= toInt64(gv_b);\n");
  VT("<?php $a &= $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a &= gv_b;\n");
  VT("<?php $a |= $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a |= gv_b;\n");
  VT("<?php $a ^= $b;",    "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a ^= gv_b;\n");
  VT("<?php $a <<= $b;",   "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a <<= toInt64(gv_b);\n");
  VT("<?php $a >>= $b;",   "Variant gv_b;\nVariant gv_a;\n\n"
     "gv_a >>= toInt64(gv_b);\n");
  VT("<?php $a || $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "(toBoolean(gv_a) || toBoolean(gv_b));\n");
  VT("<?php $a && $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "(toBoolean(gv_a) && toBoolean(gv_b));\n");
  VT("<?php $a or $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "(toBoolean(gv_a) || toBoolean(gv_b));\n");
  VT("<?php $a and $b;",   "Variant gv_a;\nVariant gv_b;\n\n"
     "(toBoolean(gv_a) && toBoolean(gv_b));\n");
  VT("<?php $a xor $b;",   "Variant gv_a;\nVariant gv_b;\n\n"
     "logical_xor(toBoolean(gv_a), toBoolean(gv_b));\n");
  VT("<?php $a | $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "bitwise_or(gv_a, gv_b);\n");
  VT("<?php $a & $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "bitwise_and(gv_a, gv_b);\n");
  VT("<?php $a ^ $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "bitwise_xor(gv_a, gv_b);\n");
  VT("<?php $a . $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "concat(toString(gv_a), toString(gv_b));\n");
  VT("<?php $a + $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "gv_a + gv_b;\n");
  VT("<?php $a - $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "gv_a - gv_b;\n");
  VT("<?php $a * $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "multiply(gv_a, gv_b);\n");
  VT("<?php $a / $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "divide(gv_a, gv_b);\n");
  VT("<?php $a % $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "toInt64(gv_a) % toInt64(gv_b);\n");
  VT("<?php $a << $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "toInt64(toInt64(gv_a)) << toInt64(gv_b);\n");
  VT("<?php $a >> $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "toInt64(toInt64(gv_a)) >> toInt64(gv_b);\n");
  VT("<?php $a === $b;",   "Variant gv_a;\nVariant gv_b;\n\n"
     "same(gv_a, gv_b);\n");
  VT("<?php $a !== $b;",   "Variant gv_a;\nVariant gv_b;\n\n"
     "!same(gv_a, gv_b);\n");
  VT("<?php $a == $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "equal(gv_a, gv_b);\n");
  VT("<?php $a != $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "!equal(gv_a, gv_b);\n");
  VT("<?php $a < $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "less(gv_a, gv_b);\n");
  VT("<?php $a <= $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "not_more(gv_a, gv_b);\n");
  VT("<?php $a > $b;",     "Variant gv_a;\nVariant gv_b;\n\n"
     "more(gv_a, gv_b);\n");
  VT("<?php $a >= $b;",    "Variant gv_a;\nVariant gv_b;\n\n"
     "not_less(gv_a, gv_b);\n");
  VT("<?php $a instanceof $b;", "Variant gv_a;\nVariant gv_b;\n\n"
     "toObject(gv_a).instanceof(toString(gv_b));\n");
  return true;
}

bool TestTransformerExpr::TestQOpExpression() {
  VT("<?php $a ? $b : $c;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "Variant gv_c;\n"
     "\n"
     "toBoolean(gv_a) ? ((Variant)gv_b) : ((Variant)gv_c);\n");
  return true;
}

bool TestTransformerExpr::TestArrayPairExpression() {
  VT("<?php array();",       "Array((ArrayElement*)NULL);\n");
  VT("<?php array($a);",
     "Variant gv_a;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a), NULL);\n");
  VT("<?php array($a, $b);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a), NEW(ArrayElement)(gv_b), NULL);\n");
  VT("<?php array($a, $b,);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a), NEW(ArrayElement)(gv_b), NULL);\n");
  VT("<?php array($a => $b);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a, gv_b), NULL);\n");
  VT("<?php array($a => $b, $c => $d);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "Variant gv_c;\n"
     "Variant gv_d;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a, gv_b), NEW(ArrayElement)(gv_c, gv_d), NULL);\n");
  VT("<?php array($a => $b, $c => $d,);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "Variant gv_c;\n"
     "Variant gv_d;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a, gv_b), NEW(ArrayElement)(gv_c, gv_d), NULL);\n");
  VT("<?php array(&$a);",
     "Variant gv_a;\n"
     "\n"
     "Array(NEW(ArrayElement)(ref(gv_a)), NULL);\n");
  VT("<?php array(&$a, &$b);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "Array(NEW(ArrayElement)(ref(gv_a)), NEW(ArrayElement)(ref(gv_b)), NULL);\n");
  VT("<?php array($a => &$b);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a, ref(gv_b)), NULL);\n");
  VT("<?php array($a => &$b, $c => &$d);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "Variant gv_c;\n"
     "Variant gv_d;\n"
     "\n"
     "Array(NEW(ArrayElement)(gv_a, ref(gv_b)), NEW(ArrayElement)(gv_c, ref(gv_d)), NULL);\n");

  VT("<?php function a() { static $a = array();}",
     "void f_a();\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Array sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = Array((ArrayElement*)NULL);\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  VT("<?php function a() { static $a = array(a);}",
     "void f_a();\n"
     "const String k_a = \"a\";\n"
     "\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Array sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = Array(NEW(ArrayElement)(k_a), NULL);\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  VT("<?php function a() { static $a = array(a, b);}",
     "void f_a();\n"
     "const String k_a = \"a\";\n"
     "const String k_b = \"b\";\n"
     "\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Array sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = Array(NEW(ArrayElement)(k_a), NEW(ArrayElement)(k_b), NULL);\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  VT("<?php function a() { static $a = array(a, b,);}",
     "void f_a();\n"
     "const String k_a = \"a\";\n"
     "const String k_b = \"b\";\n"
     "\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Array sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = Array(NEW(ArrayElement)(k_a), NEW(ArrayElement)(k_b), NULL);\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  VT("<?php function a() { static $a = array(a => b);}",
     "void f_a();\n"
     "const String k_a = \"a\";\n"
     "const String k_b = \"b\";\n"
     "\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Array sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = Array(NEW(ArrayElement)(k_a, k_b), NULL);\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  VT("<?php function a() { static $a = array(a => b, c => d);}",
     "void f_a();\n"
     "const String k_a = \"a\";\n"
     "const String k_b = \"b\";\n"
     "const String k_c = \"c\";\n"
     "const String k_d = \"d\";\n"
     "\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Array sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = Array(NEW(ArrayElement)(k_a, k_b), NEW(ArrayElement)(k_c, k_d), NULL);\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  VT("<?php function a() { static $a = array(a => b, c => d,);}",
     "void f_a();\n"
     "const String k_a = \"a\";\n"
     "const String k_b = \"b\";\n"
     "const String k_c = \"c\";\n"
     "const String k_d = \"d\";\n"
     "\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Array sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = Array(NEW(ArrayElement)(k_a, k_b), NEW(ArrayElement)(k_c, k_d), NULL);\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  return true;
}

bool TestTransformerExpr::TestClassConstantExpression() {
  VT("<?php class A { const b = 2;} function a() { static $a = A::b;}",
     "extern const int64 q_a_b;\n"
     "void f_a();\n"
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "};\n"
     "const int64 q_a_b = 2;\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static int64 sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = q_a_b;\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  VT("<?php function a() { static $a = A::b;}",
     "void f_a();\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Variant sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = throw_fatal(\"unknown class constant a::b\");\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  return true;
}

bool TestTransformerExpr::TestParameterExpression() {
  VT("<?php function a() {}",
     "void f_a();\n"
     "void f_a() {\n"
     "}\n");
  VT("<?php function a($a) {}",
     "void f_a(CVarRef v_a);\n"
     "void f_a(CVarRef v_a) {\n"
     "}\n");
  VT("<?php function a($a,$b) {}",
     "void f_a(CVarRef v_a, CVarRef v_b);\n"
     "void f_a(CVarRef v_a, CVarRef v_b) {\n"
     "}\n");
  VT("<?php function a(&$a) {}",
     "void f_a(Variant v_a);\n"
     "void f_a(Variant v_a) {\n"
     "}\n");
  VT("<?php function a(&$a,$b) {}",
     "void f_a(Variant v_a, CVarRef v_b);\n"
     "void f_a(Variant v_a, CVarRef v_b) {\n"
     "}\n");
  VT("<?php function a($a,&$b) {}",
     "void f_a(CVarRef v_a, Variant v_b);\n"
     "void f_a(CVarRef v_a, Variant v_b) {\n"
     "}\n");
  VT("<?php class TT {} function a(TT $a) {}",
     "void f_a(sp_tt v_a);\n"
     "class c_tt : public ObjectData {\n"
     "  void c_tt::init();\n"
     "};\n"
     "c_tt::c_tt() {\n"
     "}\n"
     "void c_tt::init() {\n"
     "}\n"
     "void f_a(sp_tt v_a) {\n"
     "}\n");
  VT("<?php function a(TT $a) {}",
     "void f_a(CVarRef v_a);\n"
     "void f_a(CVarRef v_a) {\n"
     "}\n");
  VT("<?php function a(array $a) {}",
     "void f_a(CArrRef v_a);\n"
     "void f_a(CArrRef v_a) {\n"
     "}\n");
  VT("<?php function a($a=1) {}",
     "void f_a(int64 v_a = 1);\n"
     "void f_a(int64 v_a /* = 1 */) {\n"
     "}\n");
  VT("<?php function a($a=1,$b) {}",
     "void f_a(int64 v_a = 1, CVarRef v_b = null);\n"
     "void f_a(int64 v_a /* = 1 */, CVarRef v_b /* = null */) {\n"
     "}\n");
  VT("<?php function a($a,$b=1) {}",
     "void f_a(CVarRef v_a, int64 v_b = 1);\n"
     "void f_a(CVarRef v_a, int64 v_b /* = 1 */) {\n"
     "}\n");

  VT("<?php function matrix($n) {"
     "  $sum = 0;"
     "  while ($n--) {"
     "    $sum += $n;"
     "  }"
     "}",
     "void f_matrix(Variant v_n);\n"
     "void f_matrix(Variant v_n) {\n"
     "  Variant v_sum;\n"
     "  \n"
     "  v_sum = 0;\n"
     "  {\n"
     "    while (toBoolean(v_n--)) {\n"
     "      {\n"
     "        v_sum += v_n;\n"
     "      }\n"
     "      goto continue1; continue1:;\n"
     "    }\n"
     "    goto break1; break1:;\n"
     "  }\n"
     "}\n");

  VT("<?php function foo($a) {"
     "  $a[1]++;"
     "}",
     "void f_foo(Sequence v_a);\n"
     "void f_foo(Sequence v_a) {\n"
     "  lval(v_a.lvalAt(1, 0x5BCA7C69B794F8CELL))++;\n"
     "}\n");

  VT("<?php function foo(&$a) {"
     "  $a[1]++;"
     "}",
     "void f_foo(Variant v_a);\n"
     "void f_foo(Variant v_a) {\n"
     "  lval(v_a.lvalAt(1, 0x5BCA7C69B794F8CELL))++;\n"
     "}\n");

  return true;
}

bool TestTransformerExpr::TestModifierExpression() {
  VT("<?php class a { public $a;}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: Variant m_a;\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     );
  VT("<?php class a { protected $a;}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: Variant m_a;\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     );
  VT("<?php class a { private $a;}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: Variant m_a;\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     );
  VT("<?php class a { static $a;}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: static Variant s_a;\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "Variant c_a::s_a;\n"
     );
  VT("<?php class a { abstract function test();}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  /* public: void t_test() = 0; */\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     );
  VT("<?php final class a { final function test() {}}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: void t_test();\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "void c_a::t_test() {\n"
     "}\n"
     );
  VT("<?php class a { public static $a;}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: static Variant s_a;\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "Variant c_a::s_a;\n"
     );
  return true;
}

bool TestTransformerExpr::TestConstant() {
  VT("<?php class a { const A = 1;}",
     "extern const int64 q_a_A;\n"
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "};\n"
     "const int64 q_a_A = 1;\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     );
  VT("<?php class a { const A=1,B=2;}",
     "extern const int64 q_a_A;\n"
     "extern const int64 q_a_B;\n"
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "};\n"
     "const int64 q_a_A = 1;\n"
     "const int64 q_a_B = 2;\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     );
  return true;
}

bool TestTransformerExpr::TestEncapsListExpression() {
  VT("<?php print '\"';",                "print(\"\\\"\");\n");
  VT("<?php print '\\'\\\\\"';",         "print(\"'\\\\\\\"\");\n");
  VT("<?php print '$a$b';",              "print(\"$a$b\");\n");
  VT("<?php print \"$a$b\";",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "print(toString(gv_a) + toString(gv_b));\n");
  VT("<?php print <<<EOM\n"
     "$a$b\n"
     "EOM;\n",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "print(toString(gv_a) + toString(gv_b) + toString(\"\\n\"));\n");
  VT("<?php `$a$b`;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "shell_exec(toString(gv_a) + toString(gv_b));\n");
  return true;
}
