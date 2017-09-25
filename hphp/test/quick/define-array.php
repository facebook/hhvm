<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function make_nonstatic_array($v1, $v2, $v3) {
  $a = [];
  $a[] = $v1;
  $a[] = $v2;
  $a[] = $v3;
  return $a;
}

function make_nonstatic_vec($v1, $v2, $v3) {
  $a = vec[];
  $a[] = $v1;
  $a[] = $v2;
  $a[] = $v3;
  return $a;
}

function make_nonstatic_dict($v1, $v2, $v3) {
  $a = dict[];
  $a[] = $v1;
  $a[] = $v2;
  $a[] = $v3;
  return $a;
}

function make_nonstatic_keyset($v1, $v2, $v3) {
  $a = keyset[];
  $a[] = $v1;
  $a[] = $v2;
  $a[] = $v3;
  return $a;
}

function test_array() {
  echo "====================== test_array ============================\n";

  define('ARRAY1', []);
  define('ARRAY2', [1, 2, 3, 4]);
  define('ARRAY3', ['a', 'b', 'c', 'd']);
  define('ARRAY4', [1, [false, null], [true, 'abc'], 1.23, []]);

  define('ARRAY5', [vec[], dict[], keyset[]]);
  define('ARRAY6', [vec[1, 2], dict['abc' => true], keyset['a', 100, 'b']]);

  define('ARRAY7', make_nonstatic_array('a', 'b', 'c'));
  define('ARRAY8', [1, make_nonstatic_array(100, 200, 300), 2]);

  var_dump(ARRAY1);
  var_dump(ARRAY2);
  var_dump(ARRAY3);
  var_dump(ARRAY4);
  var_dump(ARRAY5);
  var_dump(ARRAY6);
  var_dump(ARRAY7);
  var_dump(ARRAY8);
}

function test_bad_array() {
  echo "====================== test_bad_array ========================\n";

  $a = 123;

  define('BADARRAY1', [new stdclass()]);
  define('BADARRAY2', [&$a]);

  define('BADARRAY3', [1, [new stdclass()], 3]);
  define('BADARRAY4', [1, [&$a], 3]);

  define('BADARRAY5', [1, vec[new stdclass()], 3]);

  define('BADARRAY6', [1, dict['a' => new stdclass()], 3]);

  define('BADARRAY7', [$GLOBALS]);
  define('BADARRAY8', [1, [$GLOBALS]]);
}

function test_vec() {
  echo "====================== test_vec ==============================\n";

  define('VEC1', vec[]);
  define('VEC2', vec[1, 2, 3, 4]);
  define('VEC3', vec['a', 'b', 'c', 'd']);
  define('VEC4', vec[1, vec[false, null], vec[true, 'abc'], 1.23, vec[]]);

  define('VEC5', [[], dict[], keyset[]]);
  define('VEC6', [[1, 2], dict['abc' => true], keyset['a', 100, 'b']]);

  define('VEC7', make_nonstatic_vec('a', 'b', 'c'));
  define('VEC8', vec[1, make_nonstatic_vec(100, 200, 300), 2]);

  var_dump(VEC1);
  var_dump(VEC2);
  var_dump(VEC3);
  var_dump(VEC4);
  var_dump(VEC5);
  var_dump(VEC6);
  var_dump(VEC7);
  var_dump(VEC8);
}

function test_bad_vec() {
  echo "====================== test_bad_vec ==========================\n";

  $a = 123;

  define('BADVEC1', vec[new stdclass()]);

  define('BADVEC2', vec[1, vec[new stdclass()], 3]);

  define('BADVEC3', vec[1, [new stdclass()], 3]);
  define('BADVEC4', vec[1, [&$a], 3]);

  define('BADVEC5', vec[1, dict['a' => new stdclass()], 3]);

  define('BADVEC6', vec[$GLOBALS]);
  define('BADVEC7', vec[1, vec[$GLOBALS]]);
}

function test_dict() {
  echo "====================== test_dict =============================\n";

  define('DICT1', dict[]);
  define('DICT2', dict[100 => 1, 200 => 2, 300 => 3, 400 => 4]);
  define('DICT3', dict['key1' => 'a', 'key2' => 'b', 'key3' => 'c', 'key4' => 'd']);
  define('DICT4', dict['key1' => 500, 'key2' => 800]);
  define('DICT5', dict[100 => 'abc', 200 => 'def']);
  define('DICT6', dict['100' => 'abc', '200' => 'def', 100 => 'ghi', 200 => 'jkl']);
  define('DICT7', dict[0 => 1,
                       1 => dict['a' => false, 5 => null],
                       2 => dict[10 => true, 'z' => 'abc'],
                       3 => 1.23,
                       4 => dict[]]);
  define('DICT8', dict[0 => dict['100' => 5, 100 => 'abc'],
                       1 => dict[123 => 'abc', '123' => 10]]);

  define('DICT9', dict[100 => [], 200 => vec[], 300 => keyset[]]);
  define('DICT10', dict[100 => [1, 2], 200 => vec['abc'], 300 => keyset['a', 100, 'b']]);

  define('DICT11', make_nonstatic_dict('a', 'b', 'c'));
  define('DICT12', dict['key1' => 1, 'key2' => make_nonstatic_dict(100, 200, 300), 'key3' => 2]);

  var_dump(DICT1);
  var_dump(DICT2);
  var_dump(DICT3);
  var_dump(DICT4);
  var_dump(DICT5);
  var_dump(DICT6);
  var_dump(DICT7);
  var_dump(DICT8);
  var_dump(DICT9);
  var_dump(DICT10);
  var_dump(DICT11);
  var_dump(DICT12);
}

function test_bad_dict() {
  echo "====================== test_bad_dict =========================\n";

  $a = 123;

  define('BADDICT1', dict[100 => new stdclass()]);

  define('BADDICT2', dict[100 => 1, 200 => dict[1 => new stdclass()], 300 => 3]);

  define('BADDICT3', dict[100 => 1, 200 => [new stdclass()], 300 => 3]);
  define('BADDICT4', dict[100 => 1, 200 => [&$a], 300 => 3]);

  define('BADDICT5', dict[100 => 1, 200 => vec[new stdclass()], 300 => 3]);

  define('BADDICT6', dict[100 => $GLOBALS]);
  define('BADDICT7', [100 => 1, dict['abc' => $GLOBALS]]);
}

function test_keyset() {
  echo "====================== test_keyset ===========================\n";

  define('KEYSET1', keyset[]);
  define('KEYSET2', keyset[1, 2, 3, 4]);
  define('KEYSET3', keyset['a', 'b', 'c', 'd']);
  define('KEYSET4', keyset[1, '1', 2, '2']);

  define('KEYSET5', make_nonstatic_keyset('a', 'b', 'c'));
  define('KEYSET6', make_nonstatic_keyset(123, 456, 789));
  define('KEYSET7', make_nonstatic_keyset(100, '100', '200'));

  var_dump(KEYSET1);
  var_dump(KEYSET2);
  var_dump(KEYSET3);
  var_dump(KEYSET4);
  var_dump(KEYSET5);
  var_dump(KEYSET6);
  var_dump(KEYSET7);
}

test_array();
test_vec();
test_dict();
test_keyset();

test_bad_array();
test_bad_vec();
test_bad_dict();
