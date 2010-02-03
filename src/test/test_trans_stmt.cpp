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

#include <test/test_trans_stmt.h>

///////////////////////////////////////////////////////////////////////////////

TestTransformerStmt::TestTransformerStmt() {
  m_verbose = false;
}

bool TestTransformerStmt::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestFunctionStatement);
  RUN_TEST(TestClassStatement);
  RUN_TEST(TestInterfaceStatement);
  RUN_TEST(TestClassVariable);
  RUN_TEST(TestClassConstant);
  RUN_TEST(TestMethodStatement);
  RUN_TEST(TestStatementList);
  RUN_TEST(TestBlockStatement);
  RUN_TEST(TestIfBranchStatement);
  RUN_TEST(TestIfStatement);
  RUN_TEST(TestWhileStatement);
  RUN_TEST(TestDoStatement);
  RUN_TEST(TestForStatement);
  RUN_TEST(TestSwitchStatement);
  RUN_TEST(TestCaseStatement);
  RUN_TEST(TestBreakStatement);
  RUN_TEST(TestContinueStatement);
  RUN_TEST(TestReturnStatement);
  RUN_TEST(TestGlobalStatement);
  RUN_TEST(TestStaticStatement);
  RUN_TEST(TestEchoStatement);
  RUN_TEST(TestUnsetStatement);
  RUN_TEST(TestExpStatement);
  RUN_TEST(TestForEachStatement);
  RUN_TEST(TestCatchStatement);
  RUN_TEST(TestTryStatement);
  RUN_TEST(TestThrowStatement);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestTransformerStmt::TestFunctionStatement() {
  VT("<?php function test() {}",
     "void f_test();\n"
     "void f_test() {\n"
     "}\n");

  VT("<?php function test() {return 0;}",
     "int64 f_test();\n"
     "int64 f_test() {\n"
     "  return 0;\n"
     "}\n");

  VT("<?php function test() {return 0;return 1;}",
     "int64 f_test();\n"
     "int64 f_test() {\n"
     "  return 0;\n"
     "  return 1;\n"
     "}\n");

  VT("<?php function &test() {return 0;}",
     "Variant f_test();\n"
     "Variant f_test() {\n"
     "  return ref(0);\n"
     "}\n");

  VT("<?php function test($param) {return 0;}",
     "int64 f_test(CVarRef v_param);\n"
     "int64 f_test(CVarRef v_param) {\n"
     "  return 0;\n"
     "}\n");

  VT("<?php function test ( $param1 ,  $param2 ) {return 0;}",
     "int64 f_test(CVarRef v_param1, CVarRef v_param2);\n"
     "int64 f_test(CVarRef v_param1, CVarRef v_param2) {\n"
     "  return 0;\n"
     "}\n");

  VT("<?php "
      "function foo($a, $b)"
      "{"
      "  $a = 1;"
      "  echo ($a + $b);"
      "}"
      "foo(1, 2);",
     "void f_foo(int64 v_a, int64 v_b);\n"
     "void f_foo(int64 v_a, int64 v_b) {\n"
     "  v_a = 1;\n"
     "  echo((toString(v_a + v_b)));\n"
     "}\n"
     "f_foo(1, 2);\n");

  VT("<?php "
     "function bar($p)"
     "{"
     "  $p = 3;"
     "}"
     "function foo($a, $b)"
     "{"
     "  bar($a);"
     "  echo ($a + $b);"
     "}"
     "foo(1, 2);",
     "void f_bar(int64 v_p);\n"
     "void f_foo(int64 v_a, int64 v_b);\n"
     "void f_bar(int64 v_p) {\n"
     "  v_p = 3;\n"
     "}\n"
     "void f_foo(int64 v_a, int64 v_b) {\n"
     "  f_bar(v_a);\n"
     "  echo((toString(v_a + v_b)));\n"
     "}\n"
     "f_foo(1, 2);\n"
     );

  VT("<?php "
     "function bar(&$p)"
     "{"
     "  $p = 3;"
     "}"
     "function foo($a, $b)"
     "{"
     "  bar($a);"
     "  echo ($a + $b);"
     "}"
     "foo(1, 2);",
     "void f_bar(Variant v_p);\n"
     "void f_foo(Variant v_a, int64 v_b);\n"
     "void f_bar(Variant v_p) {\n"
     "  v_p = 3;\n"
     "}\n"
     "void f_foo(Variant v_a, int64 v_b) {\n"
     "  f_bar(ref(v_a));\n"
     "  echo((toString(v_a + v_b)));\n"
     "}\n"
     "f_foo(1, 2);\n"
     );

  VT("<?php "
     "function foo($a, $b)"
     "{"
     "  unset($a);"
     "  echo ($a + $b);"
     "}"
     "foo(1, 2);",
     "void f_foo(Variant v_a, int64 v_b);\n"
     "void f_foo(Variant v_a, int64 v_b) {\n"
     "  unset(v_a);\n"
     "  echo((toString(v_a + v_b)));\n"
     "}\n"
     "f_foo(1, 2);\n");

  VT("<?php "
     "function foo($a, $b)"
     "{"
     "  $c = 'a';"
     "  $$c = 3;"
     "  echo ($a + $b);"
     "}"
     "foo(1, 2);",
     "void f_foo(Variant v_a, Variant v_b);\n"
     "void f_foo(Variant v_a, Variant v_b) {\n"
     "  Variant v_c;\n"
     "  \n"
     "  v_c = \"a\";\n"
     "  variables->get(toString(v_c)) = 3;\n"
     "  echo((toString(v_a + v_b)));\n"
     "}\n"
     "f_foo(1, 2);\n");

  VT("<?php "
     "$arr = array('a' => '3');"
     "function foo($a, $b, $c)"
     "{"
     "  extract($b);"
     "  echo ($a + $c);"
     "}"
     "foo(1, $arr, 2);",
     "Variant gv_arr;\n"
     "\n"
     "void f_foo(Variant v_a, Variant v_b, Variant v_c);\n"
     "void f_foo(Variant v_a, Variant v_b, Variant v_c) {\n"
     "  extract(variables, toArray(v_b));\n"
     "  echo((toString(v_a + v_c)));\n"
     "}\n"
     "gv_arr = Array(NEW(ArrayElement)(\"a\", \"3\", 0x000000000002B606LL), NULL);\n"
     "f_foo(1, gv_arr, 2);\n");

  VT("<?php "
     "function foo($a, $b)"
     "{"
     "  $arr = array('a' => '3');"
     "  extract($a);"
     "  echo ($a + $b);"
     "}"
     "foo(1, 2);",
     "void f_foo(Variant v_a, Variant v_b);\n"
     "void f_foo(Variant v_a, Variant v_b) {\n"
     "  Variant v_arr;\n"
     "  \n"
     "  v_arr = Array(NEW(ArrayElement)(\"a\", \"3\", 0x000000000002B606LL), NULL);\n"
     "  extract(variables, toArray(v_a));\n"
     "  echo((toString(v_a + v_b)));\n"
     "}\n"
     "f_foo(1, 2);\n");

  return true;
}



bool TestTransformerStmt::TestClassStatement() {
  VT("<?php class Test {}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { public $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php abstract class Test { public $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php final class Test { public $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Base {} class Test extends Base { public $data;}",
     "class c_base : public ObjectData {\n"
     "  void c_base::init();\n"
     "};\n"
     "class c_test : public c_base {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_base::c_base() {\n"
     "}\n"
     "void c_base::init() {\n"
     "}\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "  c_base::init();\n"
     "}\n"
     );

  VT("<?php class Test extends Base { public $data;}",
     "class c_test {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "  c_base::init();\n"
     "}\n"
     );

  VT("<?php interface Base {} class Test implements Base { public $data;}",
     "class c_base {\n"
     "};\n"
     "class c_test : public c_base, public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php interface Base {} interface Base2 {}  class Test implements Base,Base2 { public $data;}",
     "class c_base {\n"
     "};\n"
     "class c_test : public c_base, public c_base2, public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "class c_base2 {\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { public $data; function test() {}}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "  public: void t___construct();\n"
     "  public: ObjectData *create();\n"
     "  public: ObjectData *dynCreate(const Array &params, bool init = true);\n"
     "};\n"
     "ObjectData *c_test::create() {\n"
     "  init();\n"
     "  t___construct();\n"
     "  return this;\n"
     "}\n"
     "ObjectData *c_test::dynCreate(const Array &params, bool init /* = true */) {\n"
     "  if (init) {\n"
     "    return (create());\n"
     "  } else return this;\n"
     "}\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void c_test::t___construct() {\n"
     "}\n"
     );

  VT("<?php class Test { private $data; function test() {}}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "  public: void t___construct();\n"
     "  public: ObjectData *create();\n"
     "  public: ObjectData *dynCreate(const Array &params, bool init = true);\n"
     "};\n"
     "ObjectData *c_test::create() {\n"
     "  init();\n"
     "  t___construct();\n"
     "  return this;\n"
     "}\n"
     "ObjectData *c_test::dynCreate(const Array &params, bool init /* = true */) {\n"
     "  if (init) {\n"
     "    return (create());\n"
     "  } else return this;\n"
     "}\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void c_test::t___construct() {\n"
     "}\n"
     );

  VT("<?php class Test { function test() {}}",
      "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: void t___construct();\n"
     "  public: ObjectData *create();\n"
     "  public: ObjectData *dynCreate(const Array &params, bool init = true);\n"
     "};\n"
     "ObjectData *c_test::create() {\n"
     "  init();\n"
     "  t___construct();\n"
     "  return this;\n"
     "}\n"
     "ObjectData *c_test::dynCreate(const Array &params, bool init /* = true */) {\n"
     "  if (init) {\n"
     "    return (create());\n"
     "  } else return this;\n"
     "}\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void c_test::t___construct() {\n"
     "}\n"
     );

  VT("<?php class Test { function test() { self::test2();}}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: void t___construct();\n"
     "  public: ObjectData *create();\n"
     "  public: ObjectData *dynCreate(const Array &params, bool init = true);\n"
     "};\n"
     "ObjectData *c_test::create() {\n"
     "  init();\n"
     "  t___construct();\n"
     "  return this;\n"
     "}\n"
     "ObjectData *c_test::dynCreate(const Array &params, bool init /* = true */) {\n"
     "  if (init) {\n"
     "    return (create());\n"
     "  } else return this;\n"
     "}\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void c_test::t___construct() {\n"
     "  throw_fatal(\"unknown method test::test2\", ((void*)NULL));\n"
     "}\n"
     );

  VT("<?php class Test { function test() { self::test();}}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: void t___construct();\n"
     "  public: ObjectData *create();\n"
     "  public: ObjectData *dynCreate(const Array &params, bool init = true);\n"
     "};\n"
     "ObjectData *c_test::create() {\n"
     "  init();\n"
     "  t___construct();\n"
     "  return this;\n"
     "}\n"
     "ObjectData *c_test::dynCreate(const Array &params, bool init /* = true */) {\n"
     "  if (init) {\n"
     "    return (create());\n"
     "  } else return this;\n"
     "}\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void c_test::t___construct() {\n"
     "  c_test::t___construct();\n"
     "}\n"
     );

  VT("<?php class P { function test();} class Test extends P { function test() { parent::test();}}",
     "class c_p : public ObjectData {\n"
     "  void c_p::init();\n"
     "  /* public: void t_test() = 0; */\n"
     "};\n"
     "class c_test : public c_p {\n"
     "  void c_test::init();\n"
     "  public: void t___construct();\n"
     "  public: ObjectData *create();\n"
     "  public: ObjectData *dynCreate(const Array &params, bool init = true);\n"
     "};\n"
     "c_p::c_p() {\n"
     "}\n"
     "void c_p::init() {\n"
     "}\n"
     "ObjectData *c_test::create() {\n"
     "  init();\n"
     "  t___construct();\n"
     "  return this;\n"
     "}\n"
     "ObjectData *c_test::dynCreate(const Array &params, bool init /* = true */) {\n"
     "  if (init) {\n"
     "    return (create());\n"
     "  } else return this;\n"
     "}\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "  c_p::init();\n"
     "}\n"
     "void c_test::t___construct() {\n"
     "  c_p::t_test();\n"
     "}\n"
     );

  return true;
}

bool TestTransformerStmt::TestInterfaceStatement() {
  VT("<?php interface Test {}",
     "class c_test {\n"
     "};\n"
     );

  VT("<?php interface Base {} interface Test extends Base {}",
     "class c_base {\n"
     "};\n"
     "class c_test : public c_base {\n"
     "};\n"
     );

  VT("<?php interface Test extends Base {}",
     "class c_test {\n"
     "};\n"
     );

  VT("<?php interface Base {} interface Base1 {} interface Test extends Base,Base1 {}",
     "class c_base {\n"
     "};\n"
     "class c_test : public c_base, public c_base1 {\n"
     "};\n"
     "class c_base1 {\n"
     "};\n"
     );

  VT("<?php interface Test { function test();}",
     "class c_test {\n"
     "  /* public: void t___construct() = 0; */\n"
     "};\n"
     );

  VT("<?php interface Test { function test(); function test2();}",
     "class c_test {\n"
     "  /* public: void t___construct() = 0; */\n"
     "  /* public: void t_test2() = 0; */\n"
     "};\n"
     );

  return true;
}

bool TestTransformerStmt::TestClassVariable() {
  VT("<?php class Test { public $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { protected $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { private $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { static $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: static Variant s_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "Variant c_test::s_data;\n"
     );

  VT("<?php class Test { abstract $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { final $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { private static $data;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: static Variant s_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "Variant c_test::s_data;\n"
     );

  VT("<?php class Test { private static $data=2;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: static int64 s_data;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "int64 c_test::s_data = 2;\n"
     );

  VT("<?php class Test { var $data,$data2;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "  public: Variant m_data2;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { var $data,$data2=2;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: Variant m_data;\n"
     "  public: int64 m_data2;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "  m_data2 = 2;\n"
     "}\n"
     );

  VT("<?php class Test { var $data=2,$data2;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: int64 m_data;\n"
     "  public: Variant m_data2;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "  m_data = 2;\n"
     "}\n"
     );

  VT("<?php class Test { var $data=2,$data2=3;}",
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "  public: int64 m_data;\n"
     "  public: int64 m_data2;\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "  m_data = 2;\n"
     "  m_data2 = 3;\n"
     "}\n"
     );

  return true;
}

bool TestTransformerStmt::TestClassConstant() {
  VT("<?php class Test { const DATA=1;}",
     "extern const int64 q_test_DATA;\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "const int64 q_test_DATA = 1;\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  VT("<?php class Test { const DATA=1,DATA2=2;}",
     "extern const int64 q_test_DATA;\n"
     "extern const int64 q_test_DATA2;\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "const int64 q_test_DATA = 1;\n"
     "const int64 q_test_DATA2 = 2;\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     );

  return true;
}

bool TestTransformerStmt::TestMethodStatement() {
  VT("<?php class A {function test();}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  /* public: void t_test() = 0; */\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     );

  VT("<?php class A {function test() {}}",
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

  VT("<?php class A {function test() {return 0;}}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: int64 t_test();\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "int64 c_a::t_test() {\n"
     "  return 0;\n"
     "}\n"
     );

  VT("<?php class A {function test() {return 0;return 1;}}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: int64 t_test();\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "int64 c_a::t_test() {\n"
     "  return 0;\n"
     "  return 1;\n"
     "}\n"
     );

  VT("<?php class A {function &test() {return 0;}}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: Variant t_test();\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "Variant c_a::t_test() {\n"
     "  return ref(0);\n"
     "}\n"
     );

  VT("<?php class A {function test($param) {return 0;}}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: int64 t_test(CVarRef v_param);\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "int64 c_a::t_test(CVarRef v_param) {\n"
     "  return 0;\n"
     "}\n"
     );

  VT("<?php class A {function test ( $param1 ,  $param2 ) {return 0;}}",
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: int64 t_test(CVarRef v_param1, CVarRef v_param2);\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "int64 c_a::t_test(CVarRef v_param1, CVarRef v_param2) {\n"
     "  return 0;\n"
     "}\n"
     );

  return true;
}

bool TestTransformerStmt::TestStatementList() {
  VT("<?php ",
     "");

  VT("<?php return;",
     "return;\n");

  VT("<?php function test() {} ; class Test {}",
     "void f_test();\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void f_test() {\n"
     "}\n");

  VT("<?php function test() {} ; class Test {} __halt_compiler();",
     "void f_test();\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void f_test() {\n"
     "}\n");

  VT("<?php ; __halt_compiler(); function test() {} class Test {}",
     "void f_test();\n"
     "class c_test : public ObjectData {\n"
     "  void c_test::init();\n"
     "};\n"
     "c_test::c_test() {\n"
     "}\n"
     "void c_test::init() {\n"
     "}\n"
     "void f_test() {\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestBlockStatement() {
  VT("<?php { }",
     "{\n"
     "}\n");

  VT("<?php { return;}",
     "{\n"
     "  return;\n"
     "}\n");

  VT("<?php { $a = 1;return;}",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  gv_a = 1;\n"
     "  return;\n"
     "}\n");

  VT("<?php declare(A=1) return;",
     "{\n"
     "  return;\n"
     "}\n");

  VT("<?php declare(A=1): return; enddeclare;",
     "{\n"
     "  return;\n"
     "}\n");

  VT("<?php declare(A=1): $a = 1;return; enddeclare;",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  gv_a = 1;\n"
     "  return;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestIfBranchStatement() {
  return TestIfStatement();
}

bool TestTransformerStmt::TestIfStatement() {
  VT("<?php if (5>1);",
     "if (more(5, 1)) {}\n");

  VT("<?php if (5>1) {}",
     "if (more(5, 1)) {\n"
     "}\n");

  VT("<?php if (5>1) { return;}",
     "if (more(5, 1)) {\n"
     "  return;\n"
     "}\n");

  VT("<?php if (5>1) { return; return 2;}",
     "if (more(5, 1)) {\n"
     "  return;\n"
     "  return 2;\n"
     "}\n"
     );

  VT("<?php if (5>1); else;",
     "if (more(5, 1)) {}\n");

  VT("<?php if (5>1); else {}",
     "if (more(5, 1)) {}\n"
     "else {\n"
     "}\n");

  VT("<?php if (5>1); else { return;}",
     "if (more(5, 1)) {}\n"
     "else {\n"
     "  return;\n"
     "}\n"
     );

  VT("<?php if (5>1); else { return; return 2;}",
     "if (more(5, 1)) {}\n"
     "else {\n"
     "  return;\n"
     "  return 2;\n"
     "}\n"
     );

  VT("<?php if (5>1); elseif(6>2);",
     "if (more(5, 1)) {}\n"
     "else if (more(6, 2)) {}\n");

  VT("<?php if (5>1); elseif(6>2){}",
     "if (more(5, 1)) {}\n"
     "else if (more(6, 2)) {\n"
     "}\n");

  VT("<?php if (5>1); elseif(6>2){ return;}",
     "if (more(5, 1)) {}\n"
     "else if (more(6, 2)) {\n"
     "  return;\n"
     "}\n");

  VT("<?php if (5>1); elseif(6>2){ return; return 2;}",
     "if (more(5, 1)) {}\n"
     "else if (more(6, 2)) {\n"
     "  return;\n"
     "  return 2;\n"
     "}\n"
     );

  VT("<?php if (5>1); elseif(6>2); else {$a=2;}",
     "Variant gv_a;\n"
     "\n"
     "if (more(5, 1)) {}\n"
     "else if (more(6, 2)) {}\n"
     "else {\n"
     "  gv_a = 2;\n"
     "}\n");

  VT("<?php if (5>1); elseif(6>2){} else {$a=2;}",
     "Variant gv_a;\n"
     "\n"
     "if (more(5, 1)) {}\n"
     "else if (more(6, 2)) {\n"
     "}\n"
     "else {\n"
     "  gv_a = 2;\n"
     "}\n");

  VT("<?php if (5>1); elseif(6>2){ return;} else{$a=2;}",
     "Variant gv_a;\n"
     "\n"
     "if (more(5, 1)) {}\n"
     "else if (more(6, 2)) {\n"
     "  return;\n"
     "}\n"
     "else {\n"
     "  gv_a = 2;\n"
     "}\n");

  VT("<?php if (5>1); elseif(6>2){ return; return 2;} else{$a=2;}",
     "Variant gv_a;\n"
     "\n"
     "if (more(5, 1)) {}\n"
     "else if (more(6, 2)) {\n"
     "  return;\n"
     "  return 2;\n"
     "}\n"
     "else {\n"
     "  gv_a = 2;\n"
     "}\n"
     );

  // if-endif format

  VT("<?php if (5>1): endif;",
     "if (more(5, 1)) {}\n");

  VT("<?php if (5>1):;endif;",
     "if (more(5, 1)) {\n"
     "}\n");

  VT("<?php if (5>1): {} endif;",
     "if (more(5, 1)) {\n"
     "  {\n"
     "  }\n"
     "}\n");

  VT("<?php if (5>1): return; endif;",
     "if (more(5, 1)) {\n"
     "  return;\n"
     "}\n");

  VT("<?php if (5>1): return; return 2; endif;",
     "if (more(5, 1)) {\n"
     "  return;\n"
     "  return 2;\n"
     "}\n"
     );

  VT("<?php if (5>1):; else: endif;",
     "if (more(5, 1)) {\n"
     "}\n");

  VT("<?php if (5>1):; else:; endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else {\n"
     "}\n");

  VT("<?php if (5>1):; else: {} endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else {\n"
     "  {\n"
     "  }\n"
     "}\n");

  VT("<?php if (5>1):; else: return; endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else {\n"
     "  return;\n"
     "}\n");

  VT("<?php if (5>1):; else: return; return 2; endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else {\n"
     "  return;\n"
     "  return 2;\n"
     "}\n"
     );

  VT("<?php if (5>1):; elseif(6>2): endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {}\n");

  VT("<?php if (5>1):; elseif(6>2):; endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {\n"
     "}\n");

  VT("<?php if (5>1):; elseif(6>2):{} endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {\n"
     "  {\n"
     "  }\n"
     "}\n");

  VT("<?php if (5>1):; elseif(6>2): return; endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {\n"
     "  return;\n"
     "}\n");

  VT("<?php if (5>1):; elseif(6>2): return; return 2; endif;",
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {\n"
     "  return;\n"
     "  return 2;\n"
     "}\n"
     );

  VT("<?php if (5>1):; elseif(6>2):; else: $a=2; endif;",
     "Variant gv_a;\n"
     "\n"
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {\n"
     "}\n"
     "else {\n"
     "  gv_a = 2;\n"
     "}\n");

  VT("<?php if (5>1):; elseif(6>2):{} else: $a=2; endif;",
     "Variant gv_a;\n"
     "\n"
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {\n"
     "  {\n"
     "  }\n"
     "}\n"
     "else {\n"
     "  gv_a = 2;\n"
     "}\n");

  VT("<?php if (5>1):; elseif(6>2): return; else:$a=2; endif;",
     "Variant gv_a;\n"
     "\n"
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {\n"
     "  return;\n"
     "}\n"
     "else {\n"
     "  gv_a = 2;\n"
     "}\n");

  VT("<?php if (5>1):; elseif(6>2): return; return 2; else:$a=2; endif;",
     "Variant gv_a;\n"
     "\n"
     "if (more(5, 1)) {\n"
     "}\n"
     "else if (more(6, 2)) {\n"
     "  return;\n"
     "  return 2;\n"
     "}\n"
     "else {\n"
     "  gv_a = 2;\n"
     "}\n"
     );

  return true;
}

bool TestTransformerStmt::TestWhileStatement() {
  VT("<?php while (true);",
     "{\n"
     "  while (true) {\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while (true) {}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while (true) { return;}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      return;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while (true): endwhile;",
     "{\n"
     "  while (true) {\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while (true):; endwhile;",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while (true): {} endwhile;",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      {\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while (true): return; endwhile;",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      return;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestDoStatement() {
  VT("<?php do ; while (true);",
     "{\n"
     "  do {\n"
     "    goto continue1; continue1:;\n"
     "  } while (true);\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php do {} while (true);",
     "{\n"
     "  do {\n"
     "    {\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  } while (true);\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php do {$a=1;} while (true);",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  do {\n"
     "    {\n"
     "      gv_a = 1;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  } while (true);\n"
     "  goto break1; break1:;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestForStatement() {
  VT("<?php for ($i = 1; $i < 10; $i++);",
     "Variant gv_i;\n"
     "\n"
     "{\n"
     "  for (gv_i = 1; less(gv_i, 10); gv_i++) {\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php for ($i = 1; $i < 10; $i++) {}",
     "Variant gv_i;\n"
     "\n"
     "{\n"
     "  for (gv_i = 1; less(gv_i, 10); gv_i++) {\n"
     "    {\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php for ($i = 1; $i < 10; $i++) { return;}",
     "Variant gv_i;\n"
     "\n"
     "{\n"
     "  for (gv_i = 1; less(gv_i, 10); gv_i++) {\n"
     "    {\n"
     "      return;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php for ($i = 1; $i < 10; $i++): endfor;",
     "Variant gv_i;\n"
     "\n"
     "{\n"
     "  for (gv_i = 1; less(gv_i, 10); gv_i++) {\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php for ($i = 1; $i < 10; $i++):; endfor;",
     "Variant gv_i;\n"
     "\n"
     "{\n"
     "  for (gv_i = 1; less(gv_i, 10); gv_i++) {\n"
     "    {\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php for ($i = 1; $i < 10; $i++): {} endfor;",
     "Variant gv_i;\n"
     "\n"
     "{\n"
     "  for (gv_i = 1; less(gv_i, 10); gv_i++) {\n"
     "    {\n"
     "      {\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php for ($i = 1; $i < 10; $i++): return; endfor;",
     "Variant gv_i;\n"
     "\n"
     "{\n"
     "  for (gv_i = 1; less(gv_i, 10); gv_i++) {\n"
     "    {\n"
     "      return;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php for ($i = 1,$j = 2; $i < 10; $i++,$j++): return; endfor;",
     "Variant gv_i;\n"
     "Variant gv_j;\n"
     "\n"
     "{\n"
     "  for (gv_i = 1, gv_j = 2; less(gv_i, 10); gv_i++, gv_j++) {\n"
     "    {\n"
     "      return;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestSwitchStatement() {
  VT("<?php switch($a) {}",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case 1: }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  switch (toInt64(gv_a)) {\n"
     "  case 1:\n"
     "    {\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case 1: ; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  switch (toInt64(gv_a)) {\n"
     "  case 1:\n"
     "    {\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case 1: $a = 1; break; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  switch (toInt64(gv_a)) {\n"
     "  case 1:\n"
     "    {\n"
     "      gv_a = 1;\n"
     "      break;\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case 'test': $a = 1; break; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  String tmp2 = (toString(gv_a));\n"
     "  int tmp3 = -1;\n"
     "  if (same(tmp2, (\"test\"))) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "      gv_a = 1;\n"
     "      goto break1;\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case 'test': $b = 1; break; }",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  String tmp2 = (toString(gv_a));\n"
     "  int tmp3 = -1;\n"
     "  if (same(tmp2, (\"test\"))) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "      gv_b = 1;\n"
     "      goto break1;\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { default: }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  int tmp3 = -1;\n"
     "  if (true) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { default:; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  int tmp3 = -1;\n"
     "  if (true) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { default: $a = 1; break; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  int tmp3 = -1;\n"
     "  if (true) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "      gv_a = 1;\n"
     "      goto break1;\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case 1: $a = 0; case 2: $a = 1; break; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  switch (toInt64(gv_a)) {\n"
     "  case 1:\n"
     "    {\n"
     "      gv_a = 0;\n"
     "    }\n"
     "  case 2:\n"
     "    {\n"
     "      gv_a = 1;\n"
     "      break;\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case \"test\": $a = 0; case 2: $a = 1; break; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  int tmp3 = -1;\n"
     "  if (same(tmp2, (\"test\"))) {\n"
     "    tmp3 = 0;\n"
     "  } else if (same(tmp2, (2))) {\n"
     "    tmp3 = 1;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "      gv_a = 0;\n"
     "    }\n"
     "  case 1:\n"
     "    {\n"
     "      gv_a = 1;\n"
     "      goto break1;\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case 1: $a = 0; default: $a = 1; break; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  switch (toInt64(gv_a)) {\n"
     "  case 1:\n"
     "    {\n"
     "      gv_a = 0;\n"
     "    }\n"
     "  default:\n"
     "    {\n"
     "      gv_a = 1;\n"
     "      break;\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { case 'test': $a = 0; default: $a = 1; break; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  String tmp2 = (toString(gv_a));\n"
     "  int tmp3 = -1;\n"
     "  if (same(tmp2, (\"test\"))) {\n"
     "    tmp3 = 0;\n"
     "  } else if (true) {\n"
     "    tmp3 = 1;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "      gv_a = 0;\n"
     "    }\n"
     "  case 1:\n"
     "    {\n"
     "      gv_a = 1;\n"
     "      goto break1;\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { default;; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  int tmp3 = -1;\n"
     "  if (true) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) { ;default;; }",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  int tmp3 = -1;\n"
     "  if (true) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) :default;; endswitch;",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  int tmp3 = -1;\n"
     "  if (true) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php switch($a) :;default;; endswitch;",
     "Variant gv_a;\n"
     "\n"
     "{\n"
     "  Variant tmp2 = (gv_a);\n"
     "  int tmp3 = -1;\n"
     "  if (true) {\n"
     "    tmp3 = 0;\n"
     "  }\n"
     "  switch (tmp3) {\n"
     "  case 0:\n"
     "    {\n"
     "    }\n"
     "  }\n"
     "  goto continue1; continue1:;\n"
     "  goto break1; break1:;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestCaseStatement() {
  return TestSwitchStatement();
}

bool TestTransformerStmt::TestBreakStatement() {
  VT("<?php break; break $a;",
     "Variant gv_a;\n"
     "\n"
     "throw_fatal(\"bad break\");\n"
     "throw_fatal(\"bad break\");\n");

  VT("<?php while(true) { break; }",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      break;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while(true) { break 2;}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      switch (2) {\n"
     "      case 1: goto break1;\n"
     "      default: throw_fatal(\"bad break\");\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while(true) { while(false) { break; }}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      {\n"
     "        while (false) {\n"
     "          {\n"
     "            break;\n"
     "          }\n"
     "          goto continue2; continue2:;\n"
     "        }\n"
     "        goto break2; break2:;\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while(true) { switch(1) { case 1: break; }}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      {\n"
     "        switch (1) {\n"
     "        case 1:\n"
     "          {\n"
     "            break;\n"
     "          }\n"
     "        }\n"
     "        goto continue2; continue2:;\n"
     "        goto break2; break2:;\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while(true) { switch(1) { case 1: break 2; }}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      {\n"
     "        switch (1) {\n"
     "        case 1:\n"
     "          {\n"
     "            switch (2) {\n"
     "            case 2: goto break1;\n"
     "            case 1: goto break2;\n"
     "            default: throw_fatal(\"bad break\");\n"
     "            }\n"
     "          }\n"
     "        }\n"
     "        goto continue2; continue2:;\n"
     "        goto break2; break2:;\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestContinueStatement() {
  VT("<?php continue; continue $a;",
     "Variant gv_a;\n"
     "\n"
     "throw_fatal(\"bad continue\");\n"
     "throw_fatal(\"bad continue\");\n");

  VT("<?php while(true) { continue; }",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      continue;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while(true) { continue 2;}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      switch (2) {\n"
     "      case 1: goto continue1;\n"
     "      default: throw_fatal(\"bad continue\");\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while(true) { while(false) { continue; }}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      {\n"
     "        while (false) {\n"
     "          {\n"
     "            continue;\n"
     "          }\n"
     "          goto continue2; continue2:;\n"
     "        }\n"
     "        goto break2; break2:;\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while(true) { switch(1) { case 1: continue; }}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      {\n"
     "        switch (1) {\n"
     "        case 1:\n"
     "          {\n"
     "            break;\n"
     "          }\n"
     "        }\n"
     "        goto continue2; continue2:;\n"
     "        goto break2; break2:;\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php while(true) { switch(1) { case 1: continue 2; }}",
     "{\n"
     "  while (true) {\n"
     "    {\n"
     "      {\n"
     "        switch (1) {\n"
     "        case 1:\n"
     "          {\n"
     "            switch (2) {\n"
     "            case 2: goto continue1;\n"
     "            case 1: goto continue2;\n"
     "            default: throw_fatal(\"bad continue\");\n"
     "            }\n"
     "          }\n"
     "        }\n"
     "        goto continue2; continue2:;\n"
     "        goto break2; break2:;\n"
     "      }\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestReturnStatement() {
  VT("<?php return; return $a; return 1;",
     "Variant gv_a;\n"
     "\n"
     "return;\n"
     "return gv_a;\n"
     "return 1;\n");

  return true;
}

bool TestTransformerStmt::TestGlobalStatement() {
  VT("<?php global $a;",
     "Variant gv_a;\n"
     "\n");

  VT("<?php function test() { global $$b; }",
     "void f_test();\n"
     "void f_test() {\n"
     "  throw_fatal(\"dynamic global\");\n"
     "}\n"
     );

  VT("<?php function test() { global ${$a}; }",
     "void f_test();\n"
     "void f_test() {\n"
     "  throw_fatal(\"dynamic global\");\n"
     "}\n"
     );

  VT("<?php function test() { global $a, $$b, ${$a}; }",
     "Variant gv_a;\n"
     "\n"
     "void f_test();\n"
     "void f_test() {\n"
     "  Variant v_a;\n"
     "  \n"
     "  throw_fatal(\"dynamic global\");\n"
     "}\n"
     );

  VT("<?php $a = 1; function test() { global $a; $a = 2;}",
     "Variant gv_a;\n"
     "\n"
     "void f_test();\n"
     "void f_test() {\n"
     "  // global gv_a;\n"
     "  \n"
     "  gv_a = 2;\n"
     "}\n"
     "gv_a = 1;\n");

  VT("<?php $a = 1; function test() { global $a; $a = 'test';}",
     "Variant gv_a;\n"
     "\n"
     "void f_test();\n"
     "void f_test() {\n"
     "  // global gv_a;\n"
     "  \n"
     "  gv_a = \"test\";\n"
     "}\n"
     "gv_a = 1;\n");

  VT("<?php function test() { global $a; $a = 2;}",
     "Variant gv_a;\n"
     "\n"
     "void f_test();\n"
     "void f_test() {\n"
     "  // global gv_a;\n"
     "  \n"
     "  gv_a = 2;\n"
     "}\n");

  VT("<?php function test() { global $a; $b = $a;}",
     "Variant gv_a;\n"
     "\n"
     "void f_test();\n"
     "void f_test() {\n"
     "  // global gv_a;\n"
     "  Variant v_b;\n"
     "  \n"
     "  v_b = gv_a;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestStaticStatement() {
  VT("<?php function a() { static $a; }",
     "void f_a();\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Variant sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = null;\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n");

  VT("<?php function a() { static $a = 1; }",
     "void f_a();\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static int64 sv_a;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = 1;\n"
     "    inited_a = true;\n"
     "  }\n"
     "}\n"
     );

  VT("<?php function a() { static $a, $b; }",
     "void f_a();\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Variant sv_a;\n"
     "  static bool inited_b = false; static Variant sv_b;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = null;\n"
     "    inited_a = true;\n"
     "  }\n"
     "  if (!inited_b) {\n"
     "    sv_b = null;\n"
     "    inited_b = true;\n"
     "  }\n"
     "}\n"
     );

  VT("<?php function a() { static $a = 1, $b; }",
     "void f_a();\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static int64 sv_a;\n"
     "  static bool inited_b = false; static Variant sv_b;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = 1;\n"
     "    inited_a = true;\n"
     "  }\n"
     "  if (!inited_b) {\n"
     "    sv_b = null;\n"
     "    inited_b = true;\n"
     "  }\n"
     "}\n"
     );

  VT("<?php function a() { static $a, $b = 1; }",
     "void f_a();\n"
     "void f_a() {\n"
     "  static bool inited_a = false; static Variant sv_a;\n"
     "  static bool inited_b = false; static int64 sv_b;\n"
     "  \n"
     "  if (!inited_a) {\n"
     "    sv_a = null;\n"
     "    inited_a = true;\n"
     "  }\n"
     "  if (!inited_b) {\n"
     "    sv_b = 1;\n"
     "    inited_b = true;\n"
     "  }\n"
     "}\n"
     );

  return true;
}

bool TestTransformerStmt::TestEchoStatement() {
  VT("<?php echo $a;",
     "Variant gv_a;\n"
     "\n"
     "echo(toString(gv_a));\n");

  VT("<?php echo $a, $b;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "echo(toString(gv_a));\n"
     "echo(toString(gv_b));\n");

  VT("$a;",
     "echo(\"$a;\");\n");

  VT("'\r\n"
     "\t \\$a\"",
     "echo(\"'\\r\\n"
     "\\t \\\\$a\\\"\");\n");

  return true;
}

bool TestTransformerStmt::TestUnsetStatement() {
  VT("<?php unset($a);",
     "Variant gv_a;\n"
     "\n"
     "unset(gv_a);\n");

  VT("<?php $a = 1; unset($a);",
     "Variant gv_a;\n"
     "\n"
     "gv_a = 1;\n"
     "unset(gv_a);\n");

  VT("<?php unset($a,$b);",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  unset(gv_a);\n"
     "  unset(gv_b);\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestExpStatement() {
  VT("<?php $a;",
     "Variant gv_a;\n"
     "\n"
     "gv_a;\n");

  return true;
}

bool TestTransformerStmt::TestForEachStatement() {
  VT("<?php foreach ($a as $b) ;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  Variant map2 = (toArray(gv_a));\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_b = iter3.second();\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a + $b as $b) ;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  Variant map2 = (toArray(gv_a) + toArray(gv_b));\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_b = iter3.second();\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a as $b) {}",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  Variant map2 = (toArray(gv_a));\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_b = iter3.second();\n"
     "    {\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a + $b as $b) {}",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  Variant map2 = (toArray(gv_a) + toArray(gv_b));\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_b = iter3.second();\n"
     "    {\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a as &$b) ;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  Variant map2 = ref(gv_a);\n"
     "  map2.escalate();\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_b = ref(iter3.secondRef());\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a + $b as $b => $c) ;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "Variant gv_c;\n"
     "\n"
     "{\n"
     "  Variant map2 = (toArray(gv_a) + toArray(gv_b));\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_b = iter3.first();\n"
     "    gv_c = iter3.second();\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a + $b as $b => &$c) ;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "Variant gv_c;\n"
     "\n"
     "{\n"
     "  Variant map2 = ref(toArray(gv_a) + toArray(gv_b));\n"
     "  map2.escalate();\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_b = iter3.first();\n"
     "    gv_c = ref(iter3.secondRef());\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a + $b as $b => &$c) : $a = 1; $b = 2; endforeach;",
     "Variant gv_a;\n"
     "Variant gv_b;\n"
     "Variant gv_c;\n"
     "\n"
     "{\n"
     "  Variant map2 = ref(toArray(gv_a) + toArray(gv_b));\n"
     "  map2.escalate();\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_b = iter3.first();\n"
     "    gv_c = ref(iter3.secondRef());\n"
     "    {\n"
     "      gv_a = 1;\n"
     "      gv_b = 2;\n"
     "    }\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a as &$name => &$b) ;",
     "Variant gv_a;\n"
     "Variant gv_name;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  Variant map2 = ref(gv_a);\n"
     "  map2.escalate();\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_name = iter3.first();\n"
     "    gv_b = ref(iter3.secondRef());\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  VT("<?php foreach ($a as &$name => $b) ;",
     "Variant gv_a;\n"
     "Variant gv_name;\n"
     "Variant gv_b;\n"
     "\n"
     "{\n"
     "  Variant map2 = (toArray(gv_a));\n"
     "  for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "    gv_name = iter3.first();\n"
     "    gv_b = iter3.second();\n"
     "    goto continue1; continue1:;\n"
     "  }\n"
     "  goto break1; break1:;\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestCatchStatement() {
  VT("<?php try { } catch (E $e) {}",
     "Variant gv_e;\n"
     "\n"
     "try {\n"
     "} catch (Object e) {\n"
     "  if (true) {\n"
     "    throw_fatal(\"unknown class e\");\n"
     "  }\n"
     "}\n");

  VT("<?php class E {} try { } catch (E $e) {}",
     "Variant gv_e;\n"
     "\n"
     "class c_e : public ObjectData {\n"
     "  void c_e::init();\n"
     "};\n"
     "c_e::c_e() {\n"
     "}\n"
     "void c_e::init() {\n"
     "}\n"
     "try {\n"
     "} catch (Object e) {\n"
     "  if (e.instanceof(\"e\")) {\n"
     "    gv_e = e;\n"
     "  }\n"
     "}\n");

  VT("<?php try { $a = 1;} catch (E $e) {$b=2;}",
     "Variant gv_a;\n"
     "Variant gv_e;\n"
     "Variant gv_b;\n"
     "\n"
     "try {\n"
     "  gv_a = 1;\n"
     "} catch (Object e) {\n"
     "  if (true) {\n"
     "    throw_fatal(\"unknown class e\");\n"
     "  }\n"
     "}\n");

  VT("<?php class E {} class E2 {} try { $a = 1;} catch (E $e) {$b=2;} catch (E2 $e2) { $c =3;}",
     "Variant gv_a;\n"
     "Variant gv_e;\n"
     "Variant gv_b;\n"
     "Variant gv_e2;\n"
     "Variant gv_c;\n"
     "\n"
     "class c_e : public ObjectData {\n"
     "  void c_e::init();\n"
     "};\n"
     "class c_e2 : public ObjectData {\n"
     "  void c_e2::init();\n"
     "};\n"
     "c_e::c_e() {\n"
     "}\n"
     "void c_e::init() {\n"
     "}\n"
     "c_e2::c_e2() {\n"
     "}\n"
     "void c_e2::init() {\n"
     "}\n"
     "try {\n"
     "  gv_a = 1;\n"
     "} catch (Object e) {\n"
     "  if (e.instanceof(\"e\")) {\n"
     "    gv_e = e;\n"
     "    gv_b = 2;\n"
     "  } else if (e.instanceof(\"e2\")) {\n"
     "    gv_e2 = e;\n"
     "    gv_c = 3;\n"
     "  }\n"
     "}\n");

  return true;
}

bool TestTransformerStmt::TestTryStatement() {
  return TestCatchStatement();
}

bool TestTransformerStmt::TestThrowStatement() {
  VT("<?php throw $a;",
     "Variant gv_a;\n"
     "\n"
     "throw toObject(gv_a);\n");

  return true;
}
