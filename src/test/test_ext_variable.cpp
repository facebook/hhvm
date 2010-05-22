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

#include <test/test_ext_variable.h>
#include <runtime/ext/ext_variable.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtVariable::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_is_bool);
  RUN_TEST(test_is_int);
  RUN_TEST(test_is_integer);
  RUN_TEST(test_is_long);
  RUN_TEST(test_is_double);
  RUN_TEST(test_is_float);
  RUN_TEST(test_is_numeric);
  RUN_TEST(test_is_real);
  RUN_TEST(test_is_string);
  RUN_TEST(test_is_scalar);
  RUN_TEST(test_is_array);
  RUN_TEST(test_is_object);
  RUN_TEST(test_is_resource);
  RUN_TEST(test_is_null);
  RUN_TEST(test_gettype);
  RUN_TEST(test_get_resource_type);
  RUN_TEST(test_intval);
  RUN_TEST(test_doubleval);
  RUN_TEST(test_floatval);
  RUN_TEST(test_strval);
  RUN_TEST(test_settype);
  RUN_TEST(test_print_r);
  RUN_TEST(test_var_export);
  RUN_TEST(test_var_dump);
  RUN_TEST(test_debug_zval_dump);
  RUN_TEST(test_serialize);
  RUN_TEST(test_unserialize);
  RUN_TEST(test_get_defined_vars);
  RUN_TEST(test_import_request_variables);
  RUN_TEST(test_extract);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtVariable::test_is_bool() {
  VERIFY(f_is_bool(true));
  VERIFY(!f_is_bool("test"));
  VERIFY(f_is_bool(Variant(true)));
  VERIFY(!f_is_bool(Variant("test")));
  return Count(true);
}

bool TestExtVariable::test_is_int() {
  VERIFY(f_is_int(123));
  VERIFY(!f_is_int("123"));
  VERIFY(f_is_int(Variant(123)));
  VERIFY(!f_is_int(Variant("123")));
  return Count(true);
}

bool TestExtVariable::test_is_integer() {
  VERIFY(f_is_integer(123));
  VERIFY(!f_is_integer("123"));
  VERIFY(f_is_integer(Variant(123)));
  VERIFY(!f_is_integer(Variant("123")));
  return Count(true);
}

bool TestExtVariable::test_is_long() {
  VERIFY(f_is_long(123));
  VERIFY(!f_is_long("123"));
  VERIFY(f_is_long(Variant(123)));
  VERIFY(!f_is_long(Variant("123")));
  return Count(true);
}

bool TestExtVariable::test_is_double() {
  VERIFY(f_is_double(123.4));
  VERIFY(!f_is_double("123.4"));
  VERIFY(f_is_double(Variant(123.4)));
  VERIFY(!f_is_double(Variant("123.4")));
  return Count(true);
}

bool TestExtVariable::test_is_float() {
  VERIFY(f_is_float(123.4));
  VERIFY(!f_is_float("123.4"));
  VERIFY(f_is_float(Variant(123.4)));
  VERIFY(!f_is_float(Variant("123.4")));
  return Count(true);
}

bool TestExtVariable::test_is_numeric() {
  VERIFY(f_is_numeric(123));
  VERIFY(f_is_numeric("123.4"));
  VERIFY(!f_is_numeric("e123.4"));
  VERIFY(f_is_numeric(Variant(123)));
  VERIFY(f_is_numeric(Variant("123.4")));
  return Count(true);
}

bool TestExtVariable::test_is_real() {
  VERIFY(f_is_real(123.4));
  VERIFY(!f_is_real("123.4"));
  VERIFY(f_is_real(Variant(123.4)));
  VERIFY(!f_is_real(Variant("123.4")));
  return Count(true);
}

bool TestExtVariable::test_is_string() {
  VERIFY(!f_is_string(123));
  VERIFY(f_is_string("test"));
  VERIFY(f_is_string(String("test")));
  VERIFY(f_is_string(Variant("test")));
  return Count(true);
}

bool TestExtVariable::test_is_scalar() {
  VERIFY(f_is_scalar(123));
  VERIFY(f_is_scalar("test"));
  VERIFY(!f_is_scalar(null));
  VERIFY(!f_is_scalar(CREATE_VECTOR1(123)));
  return Count(true);
}

bool TestExtVariable::test_is_array() {
  VERIFY(!f_is_array(123));
  VERIFY(!f_is_array("test"));
  VERIFY(f_is_array(CREATE_VECTOR1(123)));
  return Count(true);
}

bool TestExtVariable::test_is_object() {
  VERIFY(!f_is_object(123));
  VERIFY(!f_is_object("test"));
  VERIFY(f_is_object(Object()));
  return Count(true);
}

class TestResource : public ResourceData {
public:
  // overriding ResourceData
  const char *o_getClassName() const { return "TestResource";}
};

bool TestExtVariable::test_is_resource() {
  VERIFY(!f_is_resource(123));
  VERIFY(!f_is_resource("test"));
  VERIFY(f_is_resource(Object(new TestResource())));
  return Count(true);
}

bool TestExtVariable::test_is_null() {
  VERIFY(!f_is_null(0));
  VERIFY(!f_is_null(""));
  VERIFY(f_is_null(String()));
  VERIFY(f_is_null(Array()));
  VERIFY(f_is_null(Object()));
  VERIFY(f_is_null(Variant()));
  VERIFY(f_is_null(null));
  return Count(true);
}

bool TestExtVariable::test_gettype() {
  VS(f_gettype(null), "NULL");
  VS(f_gettype(0), "integer");
  VS(f_gettype("test"), "string");
  VS(f_gettype(String()), "string");
  VS(f_gettype(Array()), "array");
  VS(f_gettype(Object()), "object");
  return Count(true);
}

bool TestExtVariable::test_get_resource_type() {
  VS(f_get_resource_type(Object(new TestResource())), "TestResource");
  return Count(true);
}

bool TestExtVariable::test_intval() {
  VS(f_intval("12"), 12);
  VS(f_intval("12", 8), 10);
  return Count(true);
}

bool TestExtVariable::test_doubleval() {
  VS(f_doubleval("12.3"), 12.3);
  return Count(true);
}

bool TestExtVariable::test_floatval() {
  VS(f_floatval("12.3"), 12.3);
  return Count(true);
}

bool TestExtVariable::test_strval() {
  VS(f_strval(123), "123");
  return Count(true);
}

bool TestExtVariable::test_settype() {
  {
    Variant v = "5bar";
    VERIFY(f_settype(ref(v), "integer"));
    VS(v, 5);
  }
  {
    Variant v = true;
    VERIFY(f_settype(ref(v), "string"));
    VS(v, "1");
  }
  return Count(true);
}

bool TestExtVariable::test_print_r() {
  Variant v = CREATE_MAP3("a","apple","b",2,"c",CREATE_VECTOR3(1,"y",3));
  VS(f_print_r(v, true),
     "Array\n"
     "(\n"
     "    [a] => apple\n"
     "    [b] => 2\n"
     "    [c] => Array\n"
     "        (\n"
     "            [0] => 1\n"
     "            [1] => y\n"
     "            [2] => 3\n"
     "        )\n"
     "\n"
     ")\n");
  return Count(true);
}

bool TestExtVariable::test_var_export() {
  Variant v = CREATE_MAP3("a","apple","b",2,"c",CREATE_VECTOR3(1,"y",3));
  VS(f_var_export(v, true),
     "array (\n"
     "  'a' => 'apple',\n"
     "  'b' => 2,\n"
     "  'c' => \n"
     "  array (\n"
     "    0 => 1,\n"
     "    1 => 'y',\n"
     "    2 => 3,\n"
     "  ),\n"
     ")");
  return Count(true);
}

bool TestExtVariable::test_var_dump() {
  Variant v = CREATE_MAP3("a","apple","b",2,"c",CREATE_VECTOR3(1,"y",3));
  g_context->obStart();
  f_var_dump(v);
  String output = g_context->obGetContents();
  g_context->obEnd();
  VS(output,
     "array(3) {\n"
     "  [\"a\"]=>\n"
     "  string(5) \"apple\"\n"
     "  [\"b\"]=>\n"
     "  int(2)\n"
     "  [\"c\"]=>\n"
     "  array(3) {\n"
     "    [0]=>\n"
     "    int(1)\n"
     "    [1]=>\n"
     "    string(1) \"y\"\n"
     "    [2]=>\n"
     "    int(3)\n"
     "  }\n"
     "}\n");
  return Count(true);
}

bool TestExtVariable::test_debug_zval_dump() {
  Variant v = CREATE_MAP3("a","apple","b",2,"c",CREATE_VECTOR3(1,"y",3));
  g_context->obStart();
  f_debug_zval_dump(v);
  String output = g_context->obGetContents();
  g_context->obEnd();
  VS(output,
     "array(3) refcount(1){\n"
     "  [\"a\"]=>\n"
     "  string(5) \"apple\" refcount(1)\n"
     "  [\"b\"]=>\n"
     "  long(2) refcount(1)\n"
     "  [\"c\"]=>\n"
     "  array(3) refcount(1){\n"
     "    [0]=>\n"
     "    long(1) refcount(1)\n"
     "    [1]=>\n"
     "    string(1) \"y\" refcount(1)\n"
     "    [2]=>\n"
     "    long(3) refcount(1)\n"
     "  }\n"
     "}\n");
  return Count(true);
}

bool TestExtVariable::test_serialize() {
  Object obj(NEW(c_stdclass)());
  obj->o_set("name", -1, "value");
  VS(f_serialize(obj), "O:8:\"stdClass\":1:{s:4:\"name\";s:5:\"value\";}");

  Variant v = CREATE_MAP3("a","apple","b",2,"c",CREATE_VECTOR3(1,"y",3));
  VS(f_serialize(v),
     "a:3:{s:1:\"a\";s:5:\"apple\";s:1:\"b\";i:2;s:1:\"c\";a:3:{i:0;i:1;i:1;s:1:\"y\";i:2;i:3;}}");
  return Count(true);
}

bool TestExtVariable::test_unserialize() {
  {
    Variant v = f_unserialize("O:8:\"stdClass\":1:{s:4:\"name\";s:5:\"value\";}");
    VERIFY(v.is(KindOfObject));
    Object obj = v.toObject();
    VS(obj->o_getClassName(), "stdClass");
    VS(obj.o_get("name"), "value");
  }
  {
    Variant v = f_unserialize(String("O:8:\"stdClass\":1:{s:7:\"\0*\0name\";s:5:\"value\";}", 45, AttachLiteral));
    VERIFY(v.is(KindOfObject));
    Object obj = v.toObject();
    VS(obj->o_getClassName(), "stdClass");
    VS(obj.o_get("name"), "value");
  }
  {
    Variant v1 = CREATE_MAP3("a","apple","b",2,"c",CREATE_VECTOR3(1,"y",3));
    Variant v2 = f_unserialize("a:3:{s:1:\"a\";s:5:\"apple\";s:1:\"b\";i:2;s:1:\"c\";a:3:{i:0;i:1;i:1;s:1:\"y\";i:2;i:3;}}");
    VS(v1, v2);
  }
  return Count(true);
}

bool TestExtVariable::test_get_defined_vars() {
  f_get_defined_vars();
  return Count(true);
}

bool TestExtVariable::test_import_request_variables() {
  try {
    f_import_request_variables("G");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtVariable::test_extract() {
  // extract() was extensively tested in TestCodeRun.
  return Count(true);
}
