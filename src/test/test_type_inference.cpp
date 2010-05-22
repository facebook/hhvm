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

#include <test/test_type_inference.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

TestTypeInference::TestTypeInference() {
  m_verbose = false;
}

bool TestTypeInference::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestLocalVariable);
  RUN_TEST(TestDynamicLocalVariable);
  RUN_TEST(TestClassConstant);
  RUN_TEST(TestClassVariable);
  RUN_TEST(TestDynamicClassVariable);
  RUN_TEST(TestGlobalConstant);
  RUN_TEST(TestGlobalVariable);
  RUN_TEST(TestDynamicGlobalVariable);
  RUN_TEST(TestFunctionReturn);
  RUN_TEST(TestFunctionParameter);
  RUN_TEST(TestMethodParameter);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Local variables are well-scoped. If either l-dynamic variable or r-dynamic
 * variable is present, a local variable table needs to set up and all
 * local variables become Variants.
 */
bool TestTypeInference::TestLocalVariable() {
  // l-value infers types
  VT("<?php function t() { $a = 1;}",
     "void f_t();\n"
     "void f_t() {\n"
     "  int64 v_a = 0;\n"
     "  \n"
     "  v_a = 1;\n"
     "}\n");

  // two different types should coerce
  VT("<?php function t() { $a = 1; $a = 'test';}",
     "void f_t();\n"
     "void f_t() {\n"
     "  Variant v_a;\n"
     "  \n"
     "  v_a = 1;\n"
     "  v_a = \"test\";\n"
     "}\n");

  // r-value should not modify inferred type...
  VT("<?php function t() { $a = 1; print $a;}",
     "void f_t();\n"
     "void f_t() {\n"
     "  int64 v_a = 0;\n"
     "  \n"
     "  v_a = 1;\n"
     "  print(toString(v_a));\n"
     "}\n");

  // ...although it could, when some expressions require certain types.
  VT("<?php function t() { $a = 1; foreach($a as $b) {}}",
     "void f_t();\n"
     "void f_t() {\n"
     "  Variant v_a;\n"
     "  Variant v_b;\n"
     "  \n"
     "  v_a = 1;\n"
     "  {\n"
     "    Variant map2 = (toArray(v_a));\n"
     "    for (ArrayIter iter3 = map2.begin(); !iter3.end(); iter3.next()) {\n"
     "      v_b = iter3.second();\n"
     "      {\n"
     "      }\n"
     "      goto continue1; continue1:;\n"
     "    }\n"
     "    goto break1; break1:;\n"
     "  }\n"
     "}\n");

  // this would be problematic if type inference only has one pass
  VT("<?php function t() { $a = 1; while(true){ $b=$a; print $b; $a='t';}}",
     "void f_t();\n"
     "void f_t() {\n"
     "  Variant v_a;\n"
     "  Variant v_b;\n"
     "  \n"
     "  v_a = 1;\n"
     "  {\n"
     "    while (true) {\n"
     "      {\n"
     "        v_b = v_a;\n"
     "        print(toString(v_b));\n"
     "        v_a = \"t\";\n"
     "      }\n"
     "      goto continue1; continue1:;\n"
     "    }\n"
     "    goto break1; break1:;\n"
     "  }\n"
     "}\n");

  return true;
}
bool TestTypeInference::TestDynamicLocalVariable() {
  // l-dynamic variable
  VT("<?php function t() { $a = 1; $$b = 'test'; print $a;}",
     "void f_t();\n"
     "void f_t() {\n"
     "  Variant v_a;\n"
     "  Variant v_b;\n"
     "  \n"
     "  v_a = 1;\n"
     "  variables->get(toString(v_b)) = \"test\";\n"
     "  print(toString(v_a));\n"
     "}\n");

  // r-dynamic variable doesn't change inferrence results
  VT("<?php function t() { $a = 1; print $$a;}",
     "void f_t();\n"
     "void f_t() {\n"
     "  int64 v_a = 0;\n"
     "  \n"
     "  v_a = 1;\n"
     "  print(toString(variables->get(toString(v_a))));\n"
     "}\n");

  // extract() is the same as assigning values to l-dynamic variables
  VT("<?php function t($a) { extract($a);}",
     "void f_t(Variant v_a);\n"
     "void f_t(Variant v_a) {\n"
     "  extract(variables, toArray(v_a));\n"
     "}\n");

  return true;
}

bool TestTypeInference::TestClassConstant() {
  // a class constant always infers its type from its initial value
  VT("<?php class T { const a = 1; } print T::a;",
     "extern const int64 q_t_a;\n"
     "class c_t : public ObjectData {\n"
     "  void c_t::init();\n"
     "};\n"
     "const int64 q_t_a = 1;\n"
     "c_t::c_t() {\n"
     "}\n"
     "void c_t::init() {\n"
     "}\n"
     "print(toString(q_t_a));\n");

  // r-value won't modify its type
  VT("<?php class T { const a = 'test'; } T::a + 1;",
     "extern const String q_t_a;\n"
     "class c_t : public ObjectData {\n"
     "  void c_t::init();\n"
     "};\n"
     "const String q_t_a = \"test\";\n"
     "c_t::c_t() {\n"
     "}\n"
     "void c_t::init() {\n"
     "}\n"
     "(Variant)(q_t_a) + 1;\n");

  return true;
}

bool TestTypeInference::TestClassVariable() {
  // l-value infers types
  VT("<?php class T { var $a; function t() { $this->a = 1;}}",
     "class c_t : public ObjectData {\n"
     "  void c_t::init();\n"
     "  public: int64 m_a;\n"
     "  public: void t___construct();\n"
     "  public: ObjectData *create();\n"
     "  public: ObjectData *dynCreate(const Array &params, bool init = true);\n"
     "};\n"
     "ObjectData *c_t::create() {\n"
     "  init();\n"
     "  t___construct();\n"
     "  return this;\n"
     "}\n"
     "ObjectData *c_t::dynCreate(const Array &params, bool init /* = true */) {\n"
     "  if (init) {\n"
     "    return (create());\n"
     "  } else return this;\n"
     "}\n"
     "c_t::c_t() {\n"
     "}\n"
     "void c_t::init() {\n"
     "  m_a = 0;\n"
     "}\n"
     "void c_t::t___construct() {\n"
     "  m_a = 1;\n"
     "}\n"
     );

  // 1st one got a narrower type of "int64" because of 2nd pass
  VT("<?php class T { var $a;} function t() { $b = new T(); $c = $b->a;} function t2() { $b = new T(); $b->a = 1;}",
     "void f_t();\n"
     "void f_t2();\n"
     "class c_t : public ObjectData {\n"
     "  void c_t::init();\n"
     "  public: int64 m_a;\n"
     "};\n"
     "c_t::c_t() {\n"
     "}\n"
     "void c_t::init() {\n"
     "  m_a = 0;\n"
     "}\n"
     "void f_t() {\n"
     "  sp_t v_b;\n"
     "  int64 v_c = 0;\n"
     "  \n"
     "  v_b = sp_t(sp_t(NEW(c_t)())->create());\n"
     "  v_c = v_b->m_a;\n"
     "}\n"
     "void f_t2() {\n"
     "  sp_t v_b;\n"
     "  \n"
     "  v_b = sp_t(sp_t(NEW(c_t)())->create());\n"
     "  v_b->m_a = 1;\n"
     "}\n"
     );

  return true;
}

bool TestTypeInference::TestDynamicClassVariable() {
  // property table would handle this
  VT("<?php class T { var $a; function t() { $this->$a = 1;}}",
     "class c_t : public ObjectData {\n"
     "  void c_t::init();\n"
     "  public: Variant m_a;\n"
     "  public: void t___construct();\n"
     "  public: ObjectData *create();\n"
     "  public: ObjectData *dynCreate(const Array &params, bool init = true);\n"
     "};\n"
     "ObjectData *c_t::create() {\n"
     "  init();\n"
     "  t___construct();\n"
     "  return this;\n"
     "}\n"
     "ObjectData *c_t::dynCreate(const Array &params, bool init /* = true */) {\n"
     "  if (init) {\n"
     "    return (create());\n"
     "  } else return this;\n"
     "}\n"
     "c_t::c_t() {\n"
     "}\n"
     "void c_t::init() {\n"
     "}\n"
     "void c_t::t___construct() {\n"
     "  Variant v_a;\n"
     "  \n"
     "  o_lval(toString(v_a), -1LL) = 1;\n"
     "}\n"
     );

  return true;
}

/**
 * Global variables might be easier than class variables, because they are
 * always initilized with inferrable values.
 */
bool TestTypeInference::TestGlobalConstant() {
  VT("<?php define('T', 'test'); $a = T;",
     "Variant gv_a;\n"
     "\n"
     "const String k_T = \"test\";\n"
     "\n"
     ";\n"
     "gv_a = k_T;\n");

  VT("<?php apc_fetch(0, 0, A);",
     "const String k_A = \"A\";\n\n"
     "f_apc_fetch(toString(0), ref(lval(0)), toInt64(k_A));\n");

  return true;
}

bool TestTypeInference::TestGlobalVariable() {
  VT("<?php $a = 1; function t() { $b = $GLOBALS['a'];}",
     "Variant gv_a;\n"
     "\n"
     "void f_t();\n"
     "void f_t() {\n"
     "  Variant v_b;\n"
     "  \n"
     "  v_b = gv_a;\n"
     "}\n"
     "gv_a = 1;\n");

  VT("<?php function t() { $b = $GLOBALS['a'];}",
     "Variant gv_a;\n"
     "\n"
     "void f_t();\n"
     "void f_t() {\n"
     "  Variant v_b;\n"
     "  \n"
     "  v_b = gv_a;\n"
     "}\n");

  return true;
}

bool TestTypeInference::TestDynamicGlobalVariable() {
  VT("<?php $a = 1; print $$a;",
     "Variant gv_a;\n"
     "\n"
     "gv_a = 1;\n"
     "print(toString(variables->get(toString(gv_a))));\n");

  return true;
}

/**
 * The hardest type inference is actually function's, whose depend on each
 * other and multiple passes are needed.
 */
bool TestTypeInference::TestFunctionReturn() {
  VT("<?php function t() { return 1;} $a = t();",
     "Variant gv_a;\n"
     "\n"
     "int64 f_t();\n"
     "int64 f_t() {\n"
     "  return 1;\n"
     "}\n"
     "gv_a = f_t();\n");

  VT("<?php $a = t(); function t() { return r();} function r() { return 1;}",
     "Variant gv_a;\n"
     "\n"
     "int64 f_r();\n"
     "int64 f_t();\n"
     "int64 f_r() {\n"
     "  return 1;\n"
     "}\n"
     "int64 f_t() {\n"
     "  return f_r();\n"
     "}\n"
     "gv_a = f_t();\n");

  return true;
}
bool TestTypeInference::TestFunctionParameter() {
  VT("<?php function t($a) { print $a;} $a = 1; t($a); ",
     "Variant gv_a;\n"
     "\n"
     "void f_t(CVarRef v_a);\n"
     "void f_t(CVarRef v_a) {\n"
     "  print(toString(v_a));\n"
     "}\n"
     "gv_a = 1;\n"
     "f_t(gv_a);\n");

  VT("<?php function t($a) { print $a;} t(1); t('test'); ",
     "void f_t(CVarRef v_a);\n"
     "void f_t(CVarRef v_a) {\n"
     "  print(toString(v_a));\n"
     "}\n"
     "f_t(1);\n"
     "f_t(\"test\");\n");

  VT("<?php $a = 1; t($a); function t($a) { print $a;}",
     "Variant gv_a;\n"
     "\n"
     "void f_t(CVarRef v_a);\n"
     "void f_t(CVarRef v_a) {\n"
     "  print(toString(v_a));\n"
     "}\n"
     "gv_a = 1;\n"
     "f_t(gv_a);\n");

  VT("<?php $a = 1; t($a); function t($a) { r($a);} function r($a) {}",
     "Variant gv_a;\n"
     "\n"
     "void f_r(CVarRef v_a);\n"
     "void f_t(CVarRef v_a);\n"
     "void f_r(CVarRef v_a) {\n"
     "}\n"
     "void f_t(CVarRef v_a) {\n"
     "  f_r(v_a);\n"
     "}\n"
     "gv_a = 1;\n"
     "f_t(gv_a);\n");

  return true;
}

bool TestTypeInference::TestMethodParameter() {
  VT("<?php class a { function test($x) { return $x; }} $z = new a(); $z->test(1); ",
     "Variant gv_z;\n"
     "\n"
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: int64 t_test(int64 v_x);\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "int64 c_a::t_test(int64 v_x) {\n"
     "  return v_x;\n"
     "}\n"
     "gv_z = sp_a(sp_a(NEW(c_a)())->create());\n"
     "sp_a(gv_z)->t_test(1);\n"
     );

  VT("<?php class a { function test($x) { return $x; }} $z = new a(); $z->test(1); $y='test'; $z->$y(\'m\');",
     "Variant gv_z;\n"
     "Variant gv_y;\n"
     "\n"
     "class c_a : public ObjectData {\n"
     "  void c_a::init();\n"
     "  public: Variant t_test(CVarRef v_x);\n"
     "};\n"
     "c_a::c_a() {\n"
     "}\n"
     "void c_a::init() {\n"
     "}\n"
     "Variant c_a::t_test(CVarRef v_x) {\n"
     "  return v_x;\n"
     "}\n"
     "gv_z = sp_a(sp_a(NEW(c_a)())->create());\n"
     "sp_a(gv_z)->t_test(1);\n"
     "gv_y = \"test\";\n"
     "toObject(gv_z)->o_invoke((toString(gv_y)), Array(NEW(ArrayElement)(ref(\"m\")), NULL), -1LL);\n"
     );

  return true;
}
