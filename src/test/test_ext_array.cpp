/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <test/test_ext_array.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/ext/ext_array.h>
#include <runtime/ext/ext_math.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtArray::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_array_change_key_case);
  RUN_TEST(test_array_chunk);
  RUN_TEST(test_array_combine);
  RUN_TEST(test_array_count_values);
  RUN_TEST(test_array_fill_keys);
  RUN_TEST(test_array_fill);
  RUN_TEST(test_array_filter);
  RUN_TEST(test_array_flip);
  RUN_TEST(test_array_key_exists);
  RUN_TEST(test_array_keys);
  RUN_TEST(test_array_map);
  RUN_TEST(test_array_merge_recursive);
  RUN_TEST(test_array_merge);
  RUN_TEST(test_array_replace_recursive);
  RUN_TEST(test_array_replace);
  RUN_TEST(test_array_multisort);
  RUN_TEST(test_array_pad);
  RUN_TEST(test_array_pop);
  RUN_TEST(test_array_product);
  RUN_TEST(test_array_push);
  RUN_TEST(test_array_rand);
  RUN_TEST(test_array_reduce);
  RUN_TEST(test_array_reverse);
  RUN_TEST(test_array_search);
  RUN_TEST(test_array_shift);
  RUN_TEST(test_array_slice);
  RUN_TEST(test_array_splice);
  RUN_TEST(test_array_sum);
  RUN_TEST(test_array_unique);
  RUN_TEST(test_array_unshift);
  RUN_TEST(test_array_values);
  RUN_TEST(test_array_walk_recursive);
  RUN_TEST(test_array_walk);
  RUN_TEST(test_compact);
  RUN_TEST(test_shuffle);
  RUN_TEST(test_count);
  RUN_TEST(test_sizeof);
  RUN_TEST(test_each);
  RUN_TEST(test_current);
  RUN_TEST(test_next);
  RUN_TEST(test_pos);
  RUN_TEST(test_prev);
  RUN_TEST(test_reset);
  RUN_TEST(test_end);
  RUN_TEST(test_in_array);
  RUN_TEST(test_key);
  RUN_TEST(test_range);
  RUN_TEST(test_array_diff);
  RUN_TEST(test_array_udiff);
  RUN_TEST(test_array_diff_assoc);
  RUN_TEST(test_array_diff_uassoc);
  RUN_TEST(test_array_udiff_assoc);
  RUN_TEST(test_array_udiff_uassoc);
  RUN_TEST(test_array_diff_key);
  RUN_TEST(test_array_diff_ukey);
  RUN_TEST(test_array_intersect);
  RUN_TEST(test_array_uintersect);
  RUN_TEST(test_array_intersect_assoc);
  RUN_TEST(test_array_intersect_uassoc);
  RUN_TEST(test_array_uintersect_assoc);
  RUN_TEST(test_array_uintersect_uassoc);
  RUN_TEST(test_array_intersect_key);
  RUN_TEST(test_array_intersect_ukey);
  RUN_TEST(test_sort);
  RUN_TEST(test_rsort);
  RUN_TEST(test_asort);
  RUN_TEST(test_arsort);
  RUN_TEST(test_ksort);
  RUN_TEST(test_krsort);
  RUN_TEST(test_usort);
  RUN_TEST(test_uasort);
  RUN_TEST(test_uksort);
  RUN_TEST(test_natsort);
  RUN_TEST(test_natcasesort);
  RUN_TEST(test_i18n_loc_get_default);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtArray::test_array_change_key_case() {
  Array input_array = CREATE_MAP2("FirSt", 1, "SecOnd", 4);
  VS(f_print_r(f_array_change_key_case(input_array, k_CASE_UPPER), true),
     "Array\n"
     "(\n"
     "    [FIRST] => 1\n"
     "    [SECOND] => 4\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_chunk() {
  Array input_array = CREATE_VECTOR5("a", "b", "c", "d", "e");
  VS(f_print_r(f_array_chunk(input_array, 2), true),
     "Array\n"
     "(\n"
     "    [0] => Array\n"
     "        (\n"
     "            [0] => a\n"
     "            [1] => b\n"
     "        )\n"
     "\n"
     "    [1] => Array\n"
     "        (\n"
     "            [0] => c\n"
     "            [1] => d\n"
     "        )\n"
     "\n"
     "    [2] => Array\n"
     "        (\n"
     "            [0] => e\n"
     "        )\n"
     "\n"
     ")\n");
  VS(f_print_r(f_array_chunk(input_array, 2, true), true),
     "Array\n"
     "(\n"
     "    [0] => Array\n"
     "        (\n"
     "            [0] => a\n"
     "            [1] => b\n"
     "        )\n"
     "\n"
     "    [1] => Array\n"
     "        (\n"
     "            [2] => c\n"
     "            [3] => d\n"
     "        )\n"
     "\n"
     "    [2] => Array\n"
     "        (\n"
     "            [4] => e\n"
     "        )\n"
     "\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_combine() {
  Array a = CREATE_VECTOR3("green", "red", "yellow");
  Array b = CREATE_VECTOR3("avocado", "apple", "banana");
  Array c = f_array_combine(a, b);
  VS(f_print_r(c, true),
     "Array\n"
     "(\n"
     "    [green] => avocado\n"
     "    [red] => apple\n"
     "    [yellow] => banana\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_count_values() {
  Array array = CREATE_VECTOR5(1, "hello", 1, "world", "hello");
  VS(f_print_r(f_array_count_values(array), true),
     "Array\n"
     "(\n"
     "    [1] => 2\n"
     "    [hello] => 2\n"
     "    [world] => 1\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_fill_keys() {
  Array keys = CREATE_VECTOR4("foo", 5, 10, "bar");
  Array a = f_array_fill_keys(keys, "banana");
  VS(f_print_r(a, true),
     "Array\n"
     "(\n"
     "    [foo] => banana\n"
     "    [5] => banana\n"
     "    [10] => banana\n"
     "    [bar] => banana\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_fill() {
  Array a = f_array_fill(5, 6, "banana");
  Array b = f_array_fill(-2, 2, "pear");
  VS(f_print_r(a, true),
     "Array\n"
     "(\n"
     "    [5] => banana\n"
     "    [6] => banana\n"
     "    [7] => banana\n"
     "    [8] => banana\n"
     "    [9] => banana\n"
     "    [10] => banana\n"
     ")\n");
  VS(f_print_r(b, true),
     "Array\n"
     "(\n"
     "    [-2] => pear\n"
     "    [0] => pear\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_filter() {
  Array array1 = CREATE_MAP5("a", 1, "b", 2, "c", 3, "d", 4, "e", 5);
  Array array2(ArrayInit(7, true).set(6).set(7).set(8).set(9).
                                  set(10).set(11).set(12).create());

  VS(f_print_r(f_array_filter(array1, "odd"), true),
     "Array\n"
     "(\n"
     "    [a] => 1\n"
     "    [c] => 3\n"
     "    [e] => 5\n"
     ")\n");

  VS(f_print_r(f_array_filter(array2, "even"), true),
     "Array\n"
     "(\n"
     "    [0] => 6\n"
     "    [2] => 8\n"
     "    [4] => 10\n"
     "    [6] => 12\n"
     ")\n");

  Array entry = CREATE_MAP5(0, "foo", 1, false, 2, -1, 3, null, 4, "");
  VS(f_print_r(f_array_filter(entry), true),
     "Array\n"
     "(\n"
     "    [0] => foo\n"
     "    [2] => -1\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_flip() {
  Array trans = CREATE_MAP3("a", 1, "b", 1, "c", 2);
  trans = f_array_flip(trans);
  VS(f_print_r(trans, true),
     "Array\n"
     "(\n"
     "    [1] => b\n"
     "    [2] => c\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_key_exists() {
  Array search_array = CREATE_MAP2("first", 1, "second", 4);
  VERIFY(f_array_key_exists("first", search_array));

  search_array = CREATE_MAP2("first", null, "second", 4);
  //VERIFY(!isset(search_array.lvalAt("first")));
  VERIFY(f_array_key_exists("first", search_array));

  return Count(true);
}

bool TestExtArray::test_array_keys() {
  {
    Array array = CREATE_MAP2(0, 100, "color", "red");
    VS(f_print_r(f_array_keys(array), true),
       "Array\n"
       "(\n"
       "    [0] => 0\n"
       "    [1] => color\n"
       ")\n");
  }

  {
    Array array = CREATE_VECTOR5("blue", "red", "green", "blue", "blue");
    VS(f_print_r(f_array_keys(array, "blue"), true),
       "Array\n"
       "(\n"
       "    [0] => 0\n"
       "    [1] => 3\n"
       "    [2] => 4\n"
       ")\n");
  }

  {
    Array array =
      CREATE_MAP2("color", CREATE_VECTOR3("blue", "red", "green"),
                  "size", CREATE_VECTOR3("small", "medium", "large"));
    VS(f_print_r(f_array_keys(array), true),
       "Array\n"
       "(\n"
       "    [0] => color\n"
       "    [1] => size\n"
       ")\n");
  }

  return Count(true);
}

bool TestExtArray::test_array_map() {
  Array a = CREATE_VECTOR5(1, 2, 3, 4, 5);
  Array b = f_array_map(2, "cube", a);
  VS(f_print_r(b, true),
     "Array\n"
     "(\n"
     "    [0] => 1\n"
     "    [1] => 8\n"
     "    [2] => 27\n"
     "    [3] => 64\n"
     "    [4] => 125\n"
     ")\n");

  b = f_array_map(2, null, a);
  VS(f_print_r(b, true),
     "Array\n"
     "(\n"
     "    [0] => 1\n"
     "    [1] => 2\n"
     "    [2] => 3\n"
     "    [3] => 4\n"
     "    [4] => 5\n"
     ")\n");

  b = f_array_map(2, null, CREATE_MAP2("x", 6, 0, 7));
  VS(f_print_r(b, true),
     "Array\n"
     "(\n"
     "    [x] => 6\n"
     "    [0] => 7\n"
     ")\n");

  VS(f_print_r(f_array_map(3, null, CREATE_MAP2("x", 6, 0, 7),
                           CREATE_VECTOR1(CREATE_VECTOR2("a","b"))), true),
     "Array\n"
     "(\n"
     "    [0] => Array\n"
     "        (\n"
     "            [0] => 6\n"
     "            [1] => a\n"
     "        )\n"
     "\n"
     "    [1] => Array\n"
     "        (\n"
     "            [0] => 7\n"
     "            [1] => b\n"
     "        )\n"
     "\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_merge_recursive() {
  {
    Variant a1 = Array::Create();
    Variant a2 = CREATE_MAP1("key1", ref(a1));
    a1 = f_array_merge_recursive(a1, a2);
  }
  {
    Array ar1(ArrayInit(2, false).
              set("color", CREATE_MAP1("favorite", "red")).
              set(1, 5).
              create());

    Array ar2(ArrayInit(2, false).set(10).
              set("color", Array(ArrayInit(2, false).
                                 set("favorite", "green").
                                 set("blue").create())).
              create());

    Array result = f_array_merge_recursive(2, ar1, CREATE_VECTOR1(ar2));
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [color] => Array\n"
       "        (\n"
       "            [favorite] => Array\n"
       "                (\n"
       "                    [0] => red\n"
       "                    [1] => green\n"
       "                )\n"
       "\n"
       "            [0] => blue\n"
       "        )\n"
       "\n"
       "    [0] => 5\n"
       "    [1] => 10\n"
       ")\n");
  }

  return Count(true);
}

bool TestExtArray::test_array_merge() {
  {
    Array array1(ArrayInit(3, false).set("color", "red").
                                     set(2).set(4).create());
    Array array2(ArrayInit(5, false).set("a").set("b").
                                     set("color", "green").
                                     set("shape", "trapezoid").
                                     set(4).create());
    Array result = f_array_merge(2, array1, CREATE_VECTOR1(array2));
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [color] => green\n"
       "    [0] => 2\n"
       "    [1] => 4\n"
       "    [2] => a\n"
       "    [3] => b\n"
       "    [shape] => trapezoid\n"
       "    [4] => 4\n"
       ")\n");
  }
  {
    Array array1 = Array::Create();
    Array array2 = CREATE_MAP1(1, "data");
    Array result = f_array_merge(2, array1, CREATE_VECTOR1(array2));
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [0] => data\n"
       ")\n");
  }
  {
    Array array1 = Array::Create();
    Array array2 = CREATE_MAP1(1, "data");
    Array result = array1 + array2;
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [1] => data\n"
       ")\n");
  }
  {
    String beginning = "foo";
    Array end = CREATE_MAP1(1, "bar");
    Array result = f_array_merge(2, toArray(beginning), CREATE_VECTOR1(end));
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [0] => foo\n"
       "    [1] => bar\n"
       ")\n");
  }
  {
    Variant v = 1;
    Array a = CREATE_MAP1("one", 1);
    Array b = CREATE_MAP1("two", ref(v));
    Array r = f_array_merge(2, a, CREATE_VECTOR1(b));
    v = 2;
    VS(f_print_r(r, true),
       "Array\n"
       "(\n"
       "    [one] => 1\n"
       "    [two] => 2\n"
       ")\n");
  }
  {
    int64 id = 100000000000022LL;
    Array a = CREATE_MAP1(id, 1);
    Array b = CREATE_MAP1(id, 2);
    Array r = f_array_merge(2, a, CREATE_VECTOR1(b));
    VS(f_print_r(r, true),
       "Array\n"
       "(\n"
       "    [0] => 1\n"
       "    [1] => 2\n"
       ")\n");
  }
  {
    Variant a = CREATE_MAP2(1, 50, 5, 60);
    Variant b = null;
    VS(f_array_merge(2, a, CREATE_VECTOR1(b)), null);
  }
  return Count(true);
}

bool TestExtArray::test_array_replace_recursive() {
  Array ar1(ArrayInit(2, false).
            set("color", CREATE_MAP1("favorite", "red")).
            set(5).create());

  Array ar2(ArrayInit(2, false).
            set(10).
            set("color", Array(ArrayInit(2, false).
                               set("favorite", "green").
                               set("blue").create())).
            create());

  Array result = f_array_replace_recursive(2, ar1, CREATE_VECTOR1(ar2));
  VS(f_print_r(result, true),
     "Array\n"
     "(\n"
     "    [color] => Array\n"
     "        (\n"
     "            [favorite] => green\n"
     "            [0] => blue\n"
     "        )\n"
     "\n"
     "    [0] => 10\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_array_replace() {
  {
    Array array1(ArrayInit(3, false).set("color", "red").
                                     set(2).set(4).create());
    Array array2(ArrayInit(5, false).set("a").set("b").
                                     set("color", "green").
                                     set("shape", "trapezoid").
                                     set(4).create());
    Array result = f_array_replace(2, array1, CREATE_VECTOR1(array2));
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [color] => green\n"
       "    [0] => a\n"
       "    [1] => b\n"
       "    [shape] => trapezoid\n"
       "    [2] => 4\n"
       ")\n");
  }
  {
    Array array1 = Array::Create();
    Array array2 = CREATE_MAP1(1, "data");
    Array result = f_array_replace(2, array1, CREATE_VECTOR1(array2));
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [1] => data\n"
       ")\n");
  }
  {
    String beginning = "foo";
    Array end = CREATE_MAP1(1, "bar");
    Array result = f_array_replace(2, toArray(beginning), CREATE_VECTOR1(end));
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [0] => foo\n"
       "    [1] => bar\n"
       ")\n");
  }
  {
    Variant v = 1;
    Array a = CREATE_MAP1("one", 1);
    Array b = CREATE_MAP1("two", ref(v));
    Array r = f_array_replace(2, a, CREATE_VECTOR1(b));
    v = 2;
    VS(f_print_r(r, true),
       "Array\n"
       "(\n"
       "    [one] => 1\n"
       "    [two] => 2\n"
       ")\n");
  }
  {
    int64 id = 100000000000022LL;
    Array a = CREATE_MAP1(id, 1);
    Array b = CREATE_MAP1(id, 2);
    Array r = f_array_replace(2, a, CREATE_VECTOR1(b));
    VS(f_print_r(r, true),
       "Array\n"
       "(\n"
       "    [100000000000022] => 2\n"
       ")\n");
  }
  {
    Variant a = CREATE_MAP2(1, 50, 5, 60);
    Variant b = null;
    VS(f_array_replace(2, a, CREATE_VECTOR1(b)), null);
  }
  return Count(true);
}

bool TestExtArray::test_array_multisort() {
  {
    Variant ar1 = CREATE_VECTOR4(10, 100, 100, 0);
    Variant ar2 = CREATE_VECTOR4(1, 3, 2, 4);
    f_array_multisort(2, ref(ar1), CREATE_VECTOR1(ref(ar2)));
    VS(f_print_r(ar1, true),
       "Array\n"
       "(\n"
       "    [0] => 0\n"
       "    [1] => 10\n"
       "    [2] => 100\n"
       "    [3] => 100\n"
       ")\n");
    VS(f_print_r(ar2, true),
       "Array\n"
       "(\n"
       "    [0] => 4\n"
       "    [1] => 1\n"
       "    [2] => 2\n"
       "    [3] => 3\n"
       ")\n");
  }
  {
    Variant ar = CREATE_VECTOR2
      (CREATE_VECTOR5("10", 11, 100, 100, "a"),
       CREATE_VECTOR5(   1,  2, "2",   3,   1));
    f_array_multisort(6, ref(lval(ar.lvalAt(0))),
                      CREATE_VECTOR5(k_SORT_ASC, k_SORT_STRING,
                                     ref(lval(ar.lvalAt(1))),
                                     k_SORT_NUMERIC, k_SORT_DESC));
    VS(f_print_r(ar, true),
       "Array\n"
       "(\n"
       "    [0] => Array\n"
       "        (\n"
       "            [0] => 10\n"
       "            [1] => 100\n"
       "            [2] => 100\n"
       "            [3] => 11\n"
       "            [4] => a\n"
       "        )\n"
       "\n"
       "    [1] => Array\n"
       "        (\n"
       "            [0] => 1\n"
       "            [1] => 3\n"
       "            [2] => 2\n"
       "            [3] => 2\n"
       "            [4] => 1\n"
       "        )\n"
       "\n"
       ")\n");
  }
  {
    Variant array = CREATE_VECTOR4("Alpha", "atomic", "Beta", "bank");
    Variant array_lowercase = f_array_map(2, "strtolower", array);
    f_array_multisort(4, ref(array_lowercase),
                      CREATE_VECTOR3(k_SORT_ASC, k_SORT_STRING, ref(array)));
    VS(f_print_r(array, true),
       "Array\n"
       "(\n"
       "    [0] => Alpha\n"
       "    [1] => atomic\n"
       "    [2] => bank\n"
       "    [3] => Beta\n"
       ")\n");
  }

  return Count(true);
}

bool TestExtArray::test_array_pad() {
  Array input = CREATE_VECTOR3(12, 10, 9);

  VS(f_array_pad(input, 5, 0),
     CREATE_VECTOR5(12, 10, 9, 0, 0));

  VS(f_array_pad(input, -7, -1),
     Array(ArrayInit(7, true).set(-1).set(-1).set(-1).set(-1).
                              set(12).set(10).set(9).create()));

  VS(f_array_pad(input, 2, "noop"), input);

  Array a = CREATE_MAP1("9", "b");
  VS(f_array_pad(a, -3, "test"), CREATE_VECTOR3("test", "test", "b"));

  return Count(true);
}

bool TestExtArray::test_array_pop() {
  {
    Variant input = CREATE_VECTOR4("orange", "banana", "apple", "raspberry");
    String fruit = f_array_pop(ref(input));
    VS(f_print_r(input, true),
       "Array\n"
       "(\n"
       "    [0] => orange\n"
       "    [1] => banana\n"
       "    [2] => apple\n"
       ")\n");
    VS(fruit, "raspberry");
  }
  {
    Variant input = CREATE_VECTOR1("orange");
    String fruit = f_array_pop(ref(input));
    f_array_push(2, ref(input), "banana");
    VS(f_print_r(input, true),
       "Array\n"
       "(\n"
       "    [0] => banana\n"
       ")\n");
    VS(fruit, "orange");
  }

  return Count(true);
}

bool TestExtArray::test_array_product() {
  Array a = CREATE_VECTOR4(2, 4, 6, 8);
  VS(f_array_product(a), 384);
  VS(f_array_product(Array::Create()), 0);
  return Count(true);
}

bool TestExtArray::test_array_push() {
  Variant input = CREATE_VECTOR2("orange", "banana");
  Variant size =
    f_array_push(3, ref(input), "apple", CREATE_VECTOR1("raspberry"));
  VS(f_print_r(input, true),
     "Array\n"
     "(\n"
     "    [0] => orange\n"
     "    [1] => banana\n"
     "    [2] => apple\n"
     "    [3] => raspberry\n"
     ")\n");
  VS(size.toInt64(), 4);

  return Count(true);
}

bool TestExtArray::test_array_rand() {
  Array input = CREATE_VECTOR5("Neo", "Morpheus", "Trinity", "Cypher", "Tank");
  Array rand_keys = f_array_rand(input, 2);
  VS(rand_keys.size(), 2);

  std::set<std::string> strs;
  strs.insert("Neo");
  strs.insert("Morpheus");
  strs.insert("Trinity");
  strs.insert("Cypher");
  strs.insert("Tank");

  VERIFY(strs.find(input[rand_keys[0]].toString().data()) != strs.end());
  VERIFY(strs.find(input[rand_keys[1]].toString().data()) != strs.end());

  return Count(true);
}

bool TestExtArray::test_array_reduce() {
  Array a = CREATE_VECTOR5(1, 2, 3, 4, 5);
  Variant b = f_array_reduce(a, "rsum");
  VS(b, 15);
  Variant c = f_array_reduce(a, "rmul", 10);
  VS(c, 1200);
  Variant d = f_array_reduce(a, "rmul");
  VS(d, 0);

  Array x = Array::Create();
  Variant e = f_array_reduce(x, "rsum", 1);
  VS(e, 1);

  return Count(true);
}

bool TestExtArray::test_array_reverse() {
  {
    Array input = CREATE_VECTOR3("php", 4.0, CREATE_VECTOR2("green", "red"));
    Array result = f_array_reverse(input);
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [0] => Array\n"
       "        (\n"
       "            [0] => green\n"
       "            [1] => red\n"
       "        )\n"
       "\n"
       "    [1] => 4\n"
       "    [2] => php\n"
       ")\n");

    Array result_keyed = f_array_reverse(input, true);
    VS(f_print_r(result_keyed, true),
       "Array\n"
       "(\n"
       "    [2] => Array\n"
       "        (\n"
       "            [0] => green\n"
       "            [1] => red\n"
       "        )\n"
       "\n"
       "    [1] => 4\n"
       "    [0] => php\n"
       ")\n");
  }
  {
    Array input = CREATE_MAP3("php", 4.0, 10, 5.0, "blab", "b");
    Array result = f_array_reverse(input);
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [blab] => b\n"
       "    [0] => 5\n"
       "    [php] => 4\n"
       ")\n");
  }
  return Count(true);
}

bool TestExtArray::test_array_search() {
  Array array = CREATE_MAP4(0, "blue", 1, "red", 2, "green", 3, "red");
  VS(f_array_search("green", array), 2);
  VS(f_array_search("red", array), 1);
  return Count(true);
}

bool TestExtArray::test_array_shift() {
  {
    Variant input = CREATE_MAP2("a", 1, "b", 2);
    f_array_shift(ref(input));
    VS(f_print_r(input, true),
       "Array\n"
       "(\n"
       "    [b] => 2\n"
       ")\n");
  }
  {
    Variant input = CREATE_MAP2("a", 1, 23, 2);
    f_array_shift(ref(input));
    VS(f_print_r(input, true),
       "Array\n"
       "(\n"
       "    [0] => 2\n"
       ")\n");
  }
  {
    Variant input = CREATE_MAP2("a", 1, -23, 2);
    f_array_shift(ref(input));
    VS(f_print_r(input, true),
       "Array\n"
       "(\n"
       "    [0] => 2\n"
       ")\n");
  }
  {
    Variant input = CREATE_VECTOR4("orange", "banana", "apple", "raspberry");
    String fruit = f_array_shift(ref(input));
    VS(f_print_r(input, true),
       "Array\n"
       "(\n"
       "    [0] => banana\n"
       "    [1] => apple\n"
       "    [2] => raspberry\n"
       ")\n");
    VS(fruit, "orange");
  }
  return Count(true);
}

bool TestExtArray::test_array_slice() {
  Array input = CREATE_VECTOR5("a", "b", "c", "d", "e");

  VS(f_array_slice(input, 2), CREATE_VECTOR3("c", "d", "e"));
  VS(f_array_slice(input, 2, Variant()), CREATE_VECTOR3("c", "d", "e"));
  VS(f_array_slice(input, -2, 1), CREATE_VECTOR1("d"));
  VS(f_array_slice(input, 0, 3), CREATE_VECTOR3("a", "b", "c"));

  // note the differences in the array keys
  VS(f_print_r(f_array_slice(input, 2, -1), true),
     "Array\n"
     "(\n"
     "    [0] => c\n"
     "    [1] => d\n"
     ")\n");
  VS(f_print_r(f_array_slice(input, 2, -1, true), true),
     "Array\n"
     "(\n"
     "    [2] => c\n"
     "    [3] => d\n"
     ")\n");

  VS(f_array_slice(CREATE_VECTOR3("a", "b", "c"), 1, 2, true),
     CREATE_MAP2(1, "b", 2, "c"));

  VS(f_array_slice(CREATE_VECTOR3("a", "b", "c"), 1, 2, false),
     CREATE_VECTOR2("b", "c"));

  Array a = CREATE_MAP4("a", "g", 0, "a", 1, "b", 2, "c");
  a.remove("a");
  VS(f_array_slice(a, 1, 2, true),  CREATE_MAP2(1, "b", 2, "c"));
  VS(f_array_slice(a, 1, 2, false), CREATE_VECTOR2("b", "c"));

  a = CREATE_MAP4("a", 123, 0, "a", 1, "b", 2, "c");
  a.remove("a");
  VS(f_array_slice(a, 1, 2, true),  CREATE_MAP2(1, "b", 2, "c"));
  VS(f_array_slice(a, 1, 2, false), CREATE_VECTOR2("b", "c"));

  VS(f_array_slice(CREATE_VECTOR3(123, "b", "c"), 1, 2, true),
     CREATE_MAP2(1, "b", 2, "c"));

  VS(f_array_slice(CREATE_VECTOR3(123, "b", "c"), 1, 2, false),
     CREATE_VECTOR2("b", "c"));

  return Count(true);
}

bool TestExtArray::test_array_splice() {
  Variant params = CREATE_MAP2("a", "aaa", "0", "apple");
  params.remove("a");
  f_array_splice(ref(params), 0, 0, CREATE_MAP1(123, "test"));
  VS(params, CREATE_VECTOR2("test", "apple"));

  params = CREATE_MAP2("a", "aaa", "1", "apple");
  params.remove("a");
  f_array_splice(ref(params), 0, 0, CREATE_MAP1(123, "test"));
  VS(params, CREATE_VECTOR2("test", "apple"));

  Variant input = CREATE_VECTOR4("red", "green", "blue", "yellow");
  f_array_splice(ref(input), 2);
  VS(input, CREATE_VECTOR2("red", "green"));

  input = CREATE_VECTOR4("red", "green", "blue", "yellow");
  f_array_splice(ref(input), 2, Variant());
  VS(input, CREATE_VECTOR2("red", "green"));

  input = CREATE_VECTOR4("red", "green", "blue", "yellow");
  f_array_splice(ref(input), 1, -1);
  VS(input, CREATE_VECTOR2("red", "yellow"));

  input = CREATE_VECTOR4("red", "green", "blue", "yellow");
  f_array_splice(ref(input), 1, 4, "orange");
  VS(input, CREATE_VECTOR2("red", "orange"));

  input = CREATE_VECTOR4("red", "green", "blue", "yellow");
  f_array_splice(ref(input), -1, 1, CREATE_VECTOR2("black", "maroon"));
  VS(input, CREATE_VECTOR5("red", "green", "blue", "black", "maroon"));

  input = CREATE_VECTOR4("red", "green", "blue", "yellow");
  f_array_splice(ref(input), 3, 0, "purple");
  VS(input, CREATE_VECTOR5("red", "green", "blue", "purple", "yellow"));

  return Count(true);
}

bool TestExtArray::test_array_sum() {
  Array a = CREATE_VECTOR4(2, 4, 6, 8);
  VS(f_array_sum(a), 20);

  Array b = CREATE_MAP3("a", 1.2, "b", 2.3, "c", 3.4);
  VS(f_array_sum(b), 6.9);

  return Count(true);
}

bool TestExtArray::test_array_unique() {
  {
    Array input(ArrayInit(5, false).
                set("a", "green").set("red").set("b", "green").
                set("blue").set("red").create());
    Array result = f_array_unique(input);
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [a] => green\n"
       "    [0] => red\n"
       "    [1] => blue\n"
       ")\n");
  }
  {
    Array input(ArrayInit(6, true).set(4).set("4").set("3").
                                   set(4).set(3).set("3").create());
    Array result = f_array_unique(input);
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [0] => 4\n"
       "    [2] => 3\n"
       ")\n");
  }
  {
    Array input(CREATE_MAP6("a", "A", "b", "C", 0, "1",
                            2 ,"01", 1, 1, "c", "C"));
    VS(f_print_r(f_array_unique(input, k_SORT_STRING), true),
       "Array\n"
       "(\n"
       "    [a] => A\n"
       "    [b] => C\n"
       "    [0] => 1\n"
       "    [2] => 01\n"
       ")\n");
    VS(f_print_r(f_array_unique(input, k_SORT_NUMERIC), true),
       "Array\n"
       "(\n"
       "    [a] => A\n"
       "    [0] => 1\n"
       ")\n");
    VS(f_print_r(f_array_unique(input, k_SORT_REGULAR), true),
       "Array\n"
       "(\n"
       "    [a] => A\n"
       "    [b] => C\n"
       "    [0] => 1\n"
       ")\n");
  }
  {
    Array input(CREATE_MAP6(1, 1, "a", "A", "b", "C",
                            0, "1", 2 ,"01", "c", "C"));
    VS(f_print_r(f_array_unique(input, k_SORT_REGULAR), true),
       "Array\n"
       "(\n"
       "    [1] => 1\n"
       "    [a] => A\n"
       "    [b] => C\n"
       "    [0] => 1\n"
       ")\n");
  }
  return Count(true);
}

bool TestExtArray::test_array_unshift() {
  Variant q = CREATE_VECTOR2("orange", "banana");
  f_array_unshift(3, ref(q), "apple", CREATE_VECTOR1("raspberry"));
  VS(f_print_r(q, true),
     "Array\n"
     "(\n"
     "    [0] => apple\n"
     "    [1] => raspberry\n"
     "    [2] => orange\n"
     "    [3] => banana\n"
     ")\n");

  q = CREATE_MAP3(0, "orange", 1, "banana", "a", "dummy");
  q.remove("a");
  f_array_unshift(3, ref(q), "apple", CREATE_VECTOR1("raspberry"));
  VS(f_print_r(q, true),
     "Array\n"
     "(\n"
     "    [0] => apple\n"
     "    [1] => raspberry\n"
     "    [2] => orange\n"
     "    [3] => banana\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_values() {
  Array array = CREATE_MAP2("size", "XL", "color", "gold");
  VS(f_print_r(f_array_values(array), true),
     "Array\n"
     "(\n"
     "    [0] => XL\n"
     "    [1] => gold\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_array_walk_recursive() {
  Array sweet = CREATE_MAP2("a", "apple", "b", "banana");
  Array fruits = CREATE_MAP2("sweet", sweet, "sour", "lemon");

  g_context->obStart();
  f_array_walk_recursive(fruits, "test_print");
  String output = g_context->obCopyContents();
  g_context->obEnd();
  VS(output, "a: apple\nb: banana\nsour: lemon\n");

  return Count(true);
}

bool TestExtArray::test_array_walk() {
  Variant fruits = CREATE_MAP4("d", "lemon",
                               "a", "orange",
                               "b", "banana",
                               "c", "apple");
  g_context->obStart();
  f_array_walk(ref(fruits), "test_print");
  f_array_walk(ref(fruits), "test_alter", "fruit");
  f_array_walk(ref(fruits), "test_print");
  String output = g_context->obCopyContents();
  g_context->obEnd();

  VS(output,
     "d: lemon\n"
     "a: orange\n"
     "b: banana\n"
     "c: apple\n"
     "d: fruit: lemon\n"
     "a: fruit: orange\n"
     "b: fruit: banana\n"
     "c: fruit: apple\n");

  return Count(true);
}

bool TestExtArray::test_compact() {
  // compact() was tested in TestCodeRun.
  return Count(true);
}

bool TestExtArray::test_shuffle() {
  Variant numbers = f_range(1, 4);
  f_srand(5);
  f_shuffle(ref(numbers));
  Array numArr = numbers.toArray();
  VS(numArr[0], 3);
  VS(numArr[1], 4);
  VS(numArr[2], 1);
  VS(numArr[3], 2);
  return Count(true);
}

bool TestExtArray::test_count() {
  Array a;
  a.set(0, 1);
  a.set(1, 3);
  a.set(2, 5);
  VS(f_count(a), 3);

  Array b;
  b.set(0, 7);
  b.set(5, 9);
  b.set(10, 11);
  VS(f_count(b), 3);

  VS(f_count(null), 0);
  VS(f_count(false), 1);

  Array food = CREATE_MAP2("fruits",
                           CREATE_VECTOR3("orange", "banana", "apple"),
                           "veggie",
                           CREATE_VECTOR3("carrot", "collard", "pea"));
  VS(f_count(food, k_COUNT_RECURSIVE), 8);
  VS(f_count(food), 2);

  return Count(true);
}

bool TestExtArray::test_sizeof() {
  Array a;
  a.set(0, 1);
  a.set(1, 3);
  a.set(2, 5);
  VS(f_sizeof(a), 3);

  Array b;
  b.set(0, 7);
  b.set(5, 9);
  b.set(10, 11);
  VS(f_sizeof(b), 3);

  VS(f_sizeof(null), 0);
  VS(f_sizeof(false), 1);

  Array food = CREATE_MAP2("fruits",
                           CREATE_VECTOR3("orange", "banana", "apple"),
                           "veggie",
                           CREATE_VECTOR3("carrot", "collard", "pea"));
  VS(f_sizeof(food, k_COUNT_RECURSIVE), 8);
  VS(f_sizeof(food), 2);

  return Count(true);
}

bool TestExtArray::test_each() {
  {
    Variant foo = CREATE_VECTOR6("bob", "fred", "jussi", "jouni", "egon",
                                 "marliese");
    Array bar = f_each(ref(foo));
    VS(f_print_r(bar, true),
       "Array\n"
       "(\n"
       "    [1] => bob\n"
       "    [value] => bob\n"
       "    [0] => 0\n"
       "    [key] => 0\n"
       ")\n");
  }
  {
    Variant foo = CREATE_MAP2("Robert", "Bob", "Seppo", "Sepi");
    Array bar = f_each(ref(foo));
    VS(f_print_r(bar, true),
       "Array\n"
       "(\n"
       "    [1] => Bob\n"
       "    [value] => Bob\n"
       "    [0] => Robert\n"
       "    [key] => Robert\n"
       ")\n");
  }
  {
    Variant fruit = CREATE_MAP3("a", "apple", "b", "banana", "c", "cranberry");
    f_reset(ref(fruit));
    String output;
    while (true) {
      Variant item = f_each(ref(fruit));
      if (same(item, false)) break;
      output += item[0];
      output += ": ";
      output += item[1];
      output += "\n";
    }
    VS(output,
       "a: apple\n"
       "b: banana\n"
       "c: cranberry\n");
  }
  return Count(true);
}

bool TestExtArray::test_current() {
  {
    Variant transport = CREATE_VECTOR4("foot", "bike", "car", "plane");
    VS(f_current(ref(transport)), "foot");
    VS(f_next(ref(transport)), "bike");
    VS(f_current(ref(transport)), "bike");
    VS(f_prev(ref(transport)), "foot");
    VS(f_end(ref(transport)), "plane");
    VS(f_current(ref(transport)), "plane");
  }
  {
    Variant arr = Array::Create();
    VS(f_current(ref(arr)), false);
  }
  {
    Variant arr = CREATE_VECTOR1(Array::Create());
    VS(f_current(ref(arr)), Array::Create());
  }
  return Count(true);
}

bool TestExtArray::test_next() {
  Variant transport = CREATE_VECTOR4("foot", "bike", "car", "plane");
  VS(f_current(ref(transport)), "foot");
  VS(f_next(ref(transport)), "bike");
  VS(f_next(ref(transport)), "car");
  VS(f_prev(ref(transport)), "bike");
  VS(f_end(ref(transport)), "plane");

  return Count(true);
}

bool TestExtArray::test_pos() {
  {
    Variant transport = CREATE_VECTOR4("foot", "bike", "car", "plane");
    VS(f_pos(ref(transport)), "foot");
    VS(f_next(ref(transport)), "bike");
    VS(f_pos(ref(transport)), "bike");
    VS(f_prev(ref(transport)), "foot");
    VS(f_end(ref(transport)), "plane");
    VS(f_pos(ref(transport)), "plane");
  }
  {
    Variant arr = Array::Create();
    VS(f_pos(ref(arr)), false);
  }
  {
    Variant arr = CREATE_VECTOR1(Array::Create());
    VS(f_pos(ref(arr)), Array::Create());
  }
  return Count(true);
}

bool TestExtArray::test_prev() {
  Variant transport = CREATE_VECTOR4("foot", "bike", "car", "plane");
  VS(f_current(ref(transport)), "foot");
  VS(f_next(ref(transport)), "bike");
  VS(f_next(ref(transport)), "car");
  VS(f_prev(ref(transport)), "bike");
  VS(f_end(ref(transport)), "plane");

  return Count(true);
}

bool TestExtArray::test_reset() {
  Variant array = CREATE_VECTOR4("step one",
                                 "step two",
                                 "step three",
                                 "step four");

  // by default, the pointer is on the first element
  VS(f_current(ref(array)), "step one");

  // skip two steps
  f_next(ref(array));
  f_next(ref(array));
  VS(f_current(ref(array)), "step three");

  // reset pointer, start again on step one
  f_reset(ref(array));
  VS(f_current(ref(array)), "step one");

  return Count(true);
}

bool TestExtArray::test_end() {
  Variant fruits = CREATE_VECTOR3("apple", "banana", "cranberry");
  VS(f_end(ref(fruits)), "cranberry");
  return Count(true);
}

bool TestExtArray::test_in_array() {
  {
    Array os = CREATE_VECTOR4("Mac", "NT", "Irix", "Linux");
    VERIFY(f_in_array("Irix", os));
    VERIFY(!f_in_array("mac", os));
  }
  {
    Array a = CREATE_VECTOR3("1.10", 12.4, 1.13);
    VERIFY(!f_in_array("12.4", a, true));
    VERIFY(f_in_array(1.13, a, true));
  }
  {
    Array a = CREATE_VECTOR3(CREATE_VECTOR2("p", "h"),
                             CREATE_VECTOR2("p", "r"),
                             "o");
    VERIFY(f_in_array(CREATE_VECTOR2("p", "h"), a));
    VERIFY(!f_in_array(CREATE_VECTOR2("f", "i"), a));
    VERIFY(f_in_array("o", a));
  }
  return Count(true);
}

bool TestExtArray::test_key() {
  Variant array = CREATE_MAP5("fruit1", "apple",
                              "fruit2", "orange",
                              "fruit3", "grape",
                              "fruit4", "apple",
                              "fruit5", "apple");

  // this cycle echoes all associative array
  // key where value equals "apple"
  String output;
  while (true) {
    Variant fruit_name = f_current(ref(array));
    if (same(fruit_name, false)) break;
    if (same(fruit_name, "apple")) {
      output += f_key(ref(array));
    }
    f_next(ref(array));
  }
  VS(output, "fruit1fruit4fruit5");

  return Count(true);
}

bool TestExtArray::test_range() {
  Array ret = f_range(0, 12);
  VS(ret.size(), 13);
  VS(ret[0], 0);
  VS(ret[12], 12);

  // The step parameter was introduced in 5.0.0
  ret = f_range(0, 100, 10);
  VS(ret.size(), 11);
  VS(ret[0], 0);
  VS(ret[5], 50);
  VS(ret[10], 100);

  // Use of character sequences introduced in 4.1.0
  // array("a", "b", "c", "d", "e", "f", "g", "h", "i");
  ret = f_range("a", "i");
  VS(ret.size(), 9);
  VS(ret[0], "a");
  VS(ret[4], "e");
  VS(ret[8], "i");

  VS(f_range("c", "a"), CREATE_VECTOR3("c", "b", "a"));

  return Count(true);
}

bool TestExtArray::test_array_diff() {
  Array array1(ArrayInit(4, false).set("a", "green").set("red").
                                   set("blue").set("red").create());
  Array array2(ArrayInit(3, false).set("b", "green").set("yellow").
                                   set("red").create());
  Array result = f_array_diff(2, array1, array2);
  VS(f_print_r(result, true),
     "Array\n"
     "(\n"
     "    [1] => blue\n"
     ")\n");

  Array a = CREATE_VECTOR1("b");
  Array b = CREATE_VECTOR2("b", "c");
  VS(f_array_diff(2, b, a), CREATE_MAP1(1, "c"));

  return Count(true);
}

bool TestExtArray::test_array_udiff() {
  Array a = CREATE_MAP5("0.1", 9, "0.5", 12, 0, 23, 1, 4, 2, -15);
  Array b = CREATE_MAP5("0.2", 9, "0.5", 22, 0,  3, 1, 4, 2, -15);

  Array result = f_array_udiff(3, a, b, "comp_func");
  VS(f_print_r(result, true),
     "Array\n"
     "(\n"
     "    [0.5] => 12\n"
     "    [0] => 23\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_diff_assoc() {
  {
    Array array1(ArrayInit(4, false).set("a", "green").
                                     set("b", "brown").
                                     set("c", "blue").
                                     set("red").create());
    Array array2(ArrayInit(3, false).set("a", "green").
                                     set("yellow").
                                     set("red").create());
    Array result = f_array_diff_assoc(2, array1, array2);
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [b] => brown\n"
       "    [c] => blue\n"
       "    [0] => red\n"
       ")\n");
  }
  {
    Array array1 = CREATE_VECTOR3(0, 1, 2);
    Array array2 = CREATE_VECTOR3("00", "01", "2");
    Array result = f_array_diff_assoc(2, array1, array2);
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [0] => 0\n"
       "    [1] => 1\n"
       ")\n");
  }
  return Count(true);
}

bool TestExtArray::test_array_diff_uassoc() {
  {
    Array array1(ArrayInit(4, false).set("a", "green").
                                     set("b", "brown").
                                     set("c", "blue").
                                     set("red").create());
    Array array2(ArrayInit(3, false).set("a", "green").
                                     set("yellow").
                                     set("red").create());
    Array result = f_array_diff_uassoc(3, array1, array2, "comp_func");
    VS(f_print_r(result, true),
       "Array\n"
       "(\n"
       "    [b] => brown\n"
       "    [c] => blue\n"
       "    [0] => red\n"
       ")\n");
  }
  return Count(true);
}

bool TestExtArray::test_array_udiff_assoc() {
  Array a = CREATE_MAP5("0.1", 9, "0.5", 12, 0, 23, 1, 4, 2, -15);
  Array b = CREATE_MAP5("0.2", 9, "0.5", 22, 0,  3, 1, 4, 2, -15);

  Array result = f_array_udiff_assoc(3, a, b, "comp_func");
  VS(f_print_r(result, true),
     "Array\n"
     "(\n"
     "    [0.1] => 9\n"
     "    [0.5] => 12\n"
     "    [0] => 23\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_udiff_uassoc() {
  Array a = CREATE_MAP5("0.1", 9, "0.5", 12, 0, 23, 1, 4, 2, -15);
  Array b = CREATE_MAP5("0.2", 9, "0.5", 22, 0,  3, 1, 4, 2, -15);

  Array result = f_array_udiff_uassoc(4, a, b, "comp_func", "comp_func");
  VS(f_print_r(result, true),
     "Array\n"
     "(\n"
     "    [0.5] => 12\n"
     "    [0] => 23\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_diff_key() {
  Array array1 = CREATE_MAP4("blue", 1, "red", 2, "green", 3, "purple", 4);
  Array array2 = CREATE_MAP4("green", 5, "blue", 6, "yellow", 7, "cyan", 8);
  VS(f_array_diff_key(2, array1, array2),
     CREATE_MAP2("red", 2, "purple", 4));
  return Count(true);
}

bool TestExtArray::test_array_diff_ukey() {
  Array array1 = CREATE_MAP4("blue", 1, "red", 2, "green", 3, "purple", 4);
  Array array2 = CREATE_MAP4("green", 5, "blue", 6, "yellow", 7, "cyan", 8);
  VS(f_array_diff_ukey(3, array1, array2, "strcasecmp"),
     CREATE_MAP2("red", 2, "purple", 4));
  return Count(true);
}

bool TestExtArray::test_array_intersect() {
  Array array1(ArrayInit(3, false).set("a", "green").
                                   set("red").
                                   set("blue").create());
  Array array2(ArrayInit(3, false).set("b", "green").
                                   set("yellow").
                                   set("red").create());
  VS(f_array_intersect(2, array1, array2),
     CREATE_MAP2("a", "green", "0", "red"));
  return Count(true);
}

bool TestExtArray::test_array_uintersect() {
  Array array1(ArrayInit(4, false).set("a", "green").
                                   set("b", "brown").
                                   set("c", "blue").
                                   set("red").create());
  Array array2(ArrayInit(4, false).set("a", "GREEN").
                                   set("B", "brown").
                                   set("yellow").
                                   set("red").create());
  VS(f_print_r(f_array_uintersect(3, array1, array2, "strcasecmp"), true),
     "Array\n"
     "(\n"
     "    [a] => green\n"
     "    [b] => brown\n"
     "    [0] => red\n"
     ")\n");

  return Count(true);
}

bool TestExtArray::test_array_intersect_assoc() {
  Array array1(ArrayInit(4, false).set("a", "green").
                                   set("b", "brown").
                                   set("c", "blue").
                                   set("red").create());
  Array array2(ArrayInit(3, false).set("a", "green").
                                   set("yellow").
                                   set("red").create());
  VS(f_array_intersect_assoc(2, array1, array2), CREATE_MAP1("a", "green"));
  return Count(true);
}

bool TestExtArray::test_array_intersect_uassoc() {
  Array array1(ArrayInit(4, false).set("a", "green").
                                   set("b", "brown").
                                   set("c", "blue").
                                   set("red").create());
  Array array2(ArrayInit(4, false).set("a", "GREEN").
                                   set("B", "brown").
                                   set("yellow").
                                   set("red").create());
  VS(f_array_intersect_uassoc(3, array1, array2, "strcasecmp"),
     CREATE_MAP1("b", "brown"));
  return Count(true);
}

bool TestExtArray::test_array_uintersect_assoc() {
  Array array1(ArrayInit(4, false).set("a", "green").
                                   set("b", "brown").
                                   set("c", "blue").
                                   set("red").create());
  Array array2(ArrayInit(4, false).set("a", "GREEN").
                                   set("B", "brown").
                                   set("yellow").
                                   set("red").create());
  VS(f_array_uintersect_assoc(3, array1, array2, "strcasecmp"),
     CREATE_MAP1("a", "green"));
  return Count(true);
}

bool TestExtArray::test_array_uintersect_uassoc() {
  Array array1(ArrayInit(4, false).set("a", "green").
                                   set("b", "brown").
                                   set("c", "blue").
                                   set("red").create());
  Array array2(ArrayInit(4, false).set("a", "GREEN").
                                   set("B", "brown").
                                   set("yellow").
                                   set("red").create());
  VS(f_array_uintersect_uassoc(4, array1, array2, "strcasecmp", "strcasecmp"),
     CREATE_MAP2("a", "green", "b", "brown"));
  return Count(true);
}

bool TestExtArray::test_array_intersect_key() {
  Array array1 = CREATE_MAP4("blue", 1, "red", 2, "green", 3, "purple", 4);
  Array array2 = CREATE_MAP4("green", 5, "blue", 6, "yellow", 7, "cyan", 8);
  VS(f_array_intersect_key(2, array1, array2),
     CREATE_MAP2("blue", 1, "green", 3));
  VS(f_array_intersect_key(2, null, CREATE_MAP1(1, 1)), null);
  return Count(true);
}

bool TestExtArray::test_array_intersect_ukey() {
  Array array1 = CREATE_MAP4("blue", 1, "red", 2, "green", 3, "purple", 4);
  Array array2 = CREATE_MAP4("green", 5, "blue", 6, "yellow", 7, "cyan", 8);
  VS(f_array_intersect_ukey(3, array1, array2, "strcasecmp"),
     CREATE_MAP2("blue", 1, "green", 3));
  return Count(true);
}

bool TestExtArray::test_sort() {
  Variant fruits = CREATE_VECTOR4("lemon", "orange", "banana", "apple");
  f_sort(ref(fruits));
  VS(f_print_r(fruits, true),
     "Array\n"
     "(\n"
     "    [0] => apple\n"
     "    [1] => banana\n"
     "    [2] => lemon\n"
     "    [3] => orange\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_rsort() {
  Variant fruits = CREATE_VECTOR4("lemon", "orange", "banana", "apple");
  f_rsort(ref(fruits));
  VS(f_print_r(fruits, true),
     "Array\n"
     "(\n"
     "    [0] => orange\n"
     "    [1] => lemon\n"
     "    [2] => banana\n"
     "    [3] => apple\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_asort() {
  Variant fruits = CREATE_MAP4("d", "lemon", "a", "orange",
                               "b", "banana", "c", "apple");
  f_asort(ref(fruits));
  VS(f_print_r(fruits, true),
     "Array\n"
     "(\n"
     "    [c] => apple\n"
     "    [b] => banana\n"
     "    [d] => lemon\n"
     "    [a] => orange\n"
     ")\n");
  Variant arr = CREATE_VECTOR3("at", "\xe0s", "as");
  f_i18n_loc_set_default("en_US");
  f_asort(ref(arr), 0, true);
  arr = CREATE_VECTOR3("num2ber", "num1ber", "num10ber");
  f_i18n_loc_set_default("en_US");
  f_i18n_loc_set_attribute(k_UCOL_NUMERIC_COLLATION, k_UCOL_ON);
  f_i18n_loc_set_strength(k_UCOL_PRIMARY);
  f_asort(ref(arr), k_SORT_REGULAR, true);
  f_i18n_loc_set_attribute(k_UCOL_NUMERIC_COLLATION, k_UCOL_DEFAULT);
  f_i18n_loc_set_strength(k_UCOL_DEFAULT);
  VS(f_print_r(arr, true),
     "Array\n"
     "(\n"
     "    [1] => num1ber\n"
     "    [0] => num2ber\n"
     "    [2] => num10ber\n"
     ")\n");
  arr = CREATE_VECTOR4("G\xediron",        // &iacute; (Latin-1)
                       "G\xc3\xb3nzales",  // &oacute; (UTF-8)
                       "G\xc3\xa9 ara",    // &eacute; (UTF-8)
                       "G\xe1rcia");       // &aacute; (Latin-1)
  f_i18n_loc_set_default("en_US");
  f_i18n_loc_set_attribute(k_UCOL_NUMERIC_COLLATION, k_UCOL_ON);
  f_i18n_loc_set_strength(k_UCOL_PRIMARY);
  f_asort(ref(arr), k_SORT_REGULAR, true);
  f_i18n_loc_set_attribute(k_UCOL_NUMERIC_COLLATION, k_UCOL_DEFAULT);
  f_i18n_loc_set_strength(k_UCOL_DEFAULT);
  VS(f_print_r(arr, true),
     "Array\n"
     "(\n"
     "    [0] => G\xediron\n"
     "    [1] => G\xc3\xb3nzales\n"
     "    [2] => G\xc3\xa9 ara\n"
     "    [3] => G\xe1rcia\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_arsort() {
  Variant fruits = CREATE_MAP4("d", "lemon", "a", "orange",
                               "b", "banana", "c", "apple");
  f_arsort(ref(fruits));
  VS(f_print_r(fruits, true),
     "Array\n"
     "(\n"
     "    [a] => orange\n"
     "    [d] => lemon\n"
     "    [b] => banana\n"
     "    [c] => apple\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_ksort() {
  Variant fruits = CREATE_MAP4("d", "lemon", "a", "orange",
                               "b", "banana", "c", "apple");
  f_ksort(ref(fruits));
  VS(f_print_r(fruits, true),
     "Array\n"
     "(\n"
     "    [a] => orange\n"
     "    [b] => banana\n"
     "    [c] => apple\n"
     "    [d] => lemon\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_krsort() {
  Variant fruits = CREATE_MAP4("d", "lemon", "a", "orange",
                               "b", "banana", "c", "apple");
  f_krsort(ref(fruits));
  VS(f_print_r(fruits, true),
     "Array\n"
     "(\n"
     "    [d] => lemon\n"
     "    [c] => apple\n"
     "    [b] => banana\n"
     "    [a] => orange\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_usort() {
  {
    Variant a = CREATE_VECTOR5(3, 2, 5, 6, 10);
    f_usort(ref(a), "reverse_comp_func");
    VS(a, CREATE_VECTOR5(10, 6, 5, 3, 2));
  }
  return Count(true);
}

bool TestExtArray::test_uasort() {
  Variant fruits = CREATE_MAP4("d", "lemon", "a", "orange",
                               "b", "banana", "c", "apple");
  f_uasort(ref(fruits), "reverse_strcasecmp");
  VS(f_print_r(fruits, true),
     "Array\n"
     "(\n"
     "    [a] => orange\n"
     "    [d] => lemon\n"
     "    [b] => banana\n"
     "    [c] => apple\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_uksort() {
  Variant fruits = CREATE_MAP4("d", "lemon", "a", "orange",
                               "b", "banana", "c", "apple");
  f_uksort(ref(fruits), "reverse_strcasecmp");
  VS(f_print_r(fruits, true),
     "Array\n"
     "(\n"
     "    [d] => lemon\n"
     "    [c] => apple\n"
     "    [b] => banana\n"
     "    [a] => orange\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_natsort() {
  Variant array1 = CREATE_VECTOR4("img12.png",
                                  "img10.png",
                                  "img2.png",
                                  "img1.png");
  Variant array2 = array1;
  f_sort(ref(array1));
  VS(f_print_r(array1, true),
     "Array\n"
     "(\n"
     "    [0] => img1.png\n"
     "    [1] => img10.png\n"
     "    [2] => img12.png\n"
     "    [3] => img2.png\n"
     ")\n");

  f_natsort(ref(array2));
  VS(f_print_r(array2, true),
     "Array\n"
     "(\n"
     "    [3] => img1.png\n"
     "    [2] => img2.png\n"
     "    [1] => img10.png\n"
     "    [0] => img12.png\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_natcasesort() {
  Variant array1 = CREATE_VECTOR6("IMG0.png", "img12.png", "img10.png",
                                  "img2.png", "img1.png", "IMG3.png");
  Variant array2 = array1;
  f_sort(ref(array1));
  VS(f_print_r(array1, true),
     "Array\n"
     "(\n"
     "    [0] => IMG0.png\n"
     "    [1] => IMG3.png\n"
     "    [2] => img1.png\n"
     "    [3] => img10.png\n"
     "    [4] => img12.png\n"
     "    [5] => img2.png\n"
     ")\n");

  f_natcasesort(ref(array2));
  VS(f_print_r(array2, true),
     "Array\n"
     "(\n"
     "    [0] => IMG0.png\n"
     "    [4] => img1.png\n"
     "    [3] => img2.png\n"
     "    [5] => IMG3.png\n"
     "    [2] => img10.png\n"
     "    [1] => img12.png\n"
     ")\n");
  return Count(true);
}

bool TestExtArray::test_i18n_loc_get_default() {
  VERIFY(f_i18n_loc_set_default("en_UK"));
  VS(f_i18n_loc_get_default(), "en_UK");
  VERIFY(f_i18n_loc_set_default("en_US"));
  VS(f_i18n_loc_get_default(), "en_US");
  return Count(true);
}
