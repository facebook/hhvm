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

#include "hphp/test/ext/test_ext_memcached.h"
#include "hphp/runtime/ext/ext_memcached.h"
#include "hphp/runtime/ext/std/ext_std_options.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/test/ext/test_memcached_info.inc"

IMPLEMENT_SEP_EXTENSION_TEST(Memcached);
///////////////////////////////////////////////////////////////////////////////

bool TestExtMemcached::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_Memcached_construct_persistent);
  RUN_TEST(test_Memcached_get_set);
  RUN_TEST(test_Memcached_types);
  RUN_TEST(test_Memcached_cas);
  RUN_TEST(test_Memcached_delete);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

#define EXPIRATION 60
#define CREATE_MEMCACHED()                                              \
  p_Memcached memc(NEWOBJ(c_Memcached));                                \
  memc->t___construct();                                                \
  memc->t_addserver(TEST_MEMCACHED_HOSTNAME, TEST_MEMCACHED_PORT);      \
  Variant memc_version = memc->t_getversion();                          \
  if (same(memc_version, false)) {                                      \
    SKIP("No memcached running");                                       \
    return Count(true);                                                 \
  }

bool TestExtMemcached::test_Memcached_construct_persistent() {
  p_Memcached memc1(p_Memcached(NEWOBJ(c_Memcached)));
  memc1->t___construct("test");
  memc1->t_setoption(q_Memcached$$OPT_PREFIX_KEY, "php");
  VS(memc1->t_getoption(q_Memcached$$OPT_PREFIX_KEY), "php");

  p_Memcached memc2(p_Memcached(NEWOBJ(c_Memcached)));
  memc2->t___construct("test");
  VS(memc2->t_getoption(q_Memcached$$OPT_PREFIX_KEY), "php");

  p_Memcached memc3(p_Memcached(NEWOBJ(c_Memcached)));
  memc3->t___construct();
  VS(memc3->t_getoption(q_Memcached$$OPT_PREFIX_KEY), "");

  return Count(true);
}

bool TestExtMemcached::test_Memcached_get_set() {
  CREATE_MEMCACHED();

  const char *key = "foo";
  Array value = make_map_array("foo", "bar");
  memc->t_set(key, value, EXPIRATION);
  VS(memc->t_get(key), value);

  return Count(true);
}

static const StaticString s_boolean_true("boolean_true");
static const StaticString s_boolean_false("boolean_false");
static const StaticString s_string("string");
static const StaticString s_string_empty("string_empty");
static const StaticString
       s_integer_positive_integer("integer_positive_integer");
static const StaticString
       s_integer_negative_integer("integer_negative_integer");
static const StaticString s_integer_zero_integer("integer_zero_integer");
static const StaticString s_float_positive1("float_positive1");
static const StaticString s_float_positive2("float_positive2");
static const StaticString s_float_negative("float_negative");
static const StaticString s_float_zero("float_zero");
static const StaticString s_null("null");
static const StaticString s_array_empty("array_empty");
static const StaticString s_array("array");

bool TestExtMemcached::test_Memcached_types() {
  Array list;
  list.add(s_boolean_true, true);
  list.add(s_boolean_false, false);
  list.add(s_string, "just a string");
  list.add(s_string_empty, empty_string);
  list.add(s_integer_positive_integer, 10);
  list.add(s_integer_negative_integer, -10);
  list.add(s_integer_zero_integer, 0);
  list.add(s_float_positive1, 3.912131);
  list.add(s_float_positive2, 1.2131E+52);
  list.add(s_float_negative, -42.123312);
  list.add(s_float_zero, 0.0);
  list.add(s_null, uninit_null());
  list.add(s_array_empty, Array());
  list.add(s_array, make_packed_array(1, 2, 3, "foo"));

  CREATE_MEMCACHED();
  for (ArrayIter iter(list); iter; ++iter) {
    VERIFY(memc->t_set(iter.first(), iter.second(), EXPIRATION));
    VS(memc->t_get(iter.first()), iter.second());
  }

  for (ArrayIter iter(list); iter; ++iter) {
    VERIFY(memc->t_delete(iter.first()));
  }

  VERIFY(memc->t_setmulti(list, EXPIRATION));
  Variant res = memc->t_getmulti(f_array_keys(list).toArray());
  VERIFY(res.isArray());
  Array resArray = res.toArray();
  VERIFY(resArray->size() == list.size());
  for (ArrayIter iter(resArray); iter; ++iter) {
    VS(iter.second(), list[iter.first()]);
  }

  return Count(true);
}

bool TestExtMemcached::test_Memcached_cas() {
  CREATE_MEMCACHED();
  for (ArrayIter iter(memc_version.toArray()); iter; ++iter) {
    if (!HHVM_FN(version_compare)(iter.second().toString(), "1.3.0", ">=")
        .toBoolean()) {
      SKIP("Need memcached 1.3.0 for CAS");
      return Count(true);
    }
  }

  const char *key = "cas_test";

  VERIFY(memc->t_set(key, 10, EXPIRATION));

  Variant cas;

  VS(memc->t_get(key, uninit_null(), strongBind(cas)), 10);

  VERIFY(!cas.isNull() && cas.isDouble());
  VERIFY(memc->t_cas(cas.toDouble(), key, 11, EXPIRATION));

  VS(memc->t_get(key, uninit_null(), cas), 11);
  VERIFY(!memc->t_cas(cas.toDouble(), key, 12, EXPIRATION));
  VS(memc->t_get(key, uninit_null(), cas), 11);

  return Count(true);
}

bool TestExtMemcached::test_Memcached_delete() {
  CREATE_MEMCACHED();
  const char *key = "delete_test";
  VERIFY(memc->t_set(key, "foo", EXPIRATION));
  VERIFY(memc->t_delete(key));
  VS(memc->t_get(key), false);

  return Count(true);
}
