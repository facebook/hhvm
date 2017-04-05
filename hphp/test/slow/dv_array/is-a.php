<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function boolstr($b) { return $b ? "TRUE" : "FALSE"; }

function direct_test_dv_arrays($v) {
  echo "============= direct_test_dv_arrays ================\n";
  echo "value => ";
  var_dump($v);
  echo "is_varray_or_darray => " . boolstr(is_varray_or_darray($v)) . "\n";
  echo "gettype => " . gettype($v) . "\n";
}

function indirect_test_dv_arrays($v, $tests) {
  echo "============= indirect_test_dv_arrays ==============\n";
  echo "value => ";
  var_dump($v);
  foreach ($tests as $c) {
    echo "$c => " . (($c === "gettype") ? $c($v) : boolstr($c($v))) . "\n";
  }
}

function direct_test_others($v) {
  echo "============= direct_test_others ===================\n";
  echo "value => ";
  var_dump($v);
  echo "is_null => " . boolstr(is_null($v)) . "\n";
  echo "is_bool => " . boolstr(is_bool($v)) . "\n";
  echo "is_int => " . boolstr(is_int($v)) . "\n";
  echo "is_string => " . boolstr(is_string($v)) . "\n";
  echo "is_float => " . boolstr(is_float($v)) . "\n";
  echo "is_resource => " . boolstr(is_resource($v)) . "\n";
  echo "is_object => " . boolstr(is_object($v)) . "\n";
  echo "is_array => " . boolstr(is_array($v)) . "\n";
  echo "is_vec => " . boolstr(is_vec($v)) . "\n";
  echo "is_dict => " . boolstr(is_dict($v)) . "\n";
  echo "is_keyset => " . boolstr(is_keyset($v)) . "\n";
  echo "is_scalar => " . boolstr(is_scalar($v)) . "\n";
}

function indirect_test_others($v, $tests) {
  echo "============= indirect_test_others =================\n";
  echo "value => ";
  var_dump($v);
  foreach ($tests as $c) { echo "$c => " . boolstr($c($v)) . "\n"; }
}

function test1() {
  $values = vec[
    null,
    false,
    true,
    123,
    'abc',
    3.14,
    new stdclass(),
    xml_parser_create(),
    vec[],
    vec[1, 2, 3],
    dict[],
    dict['abc' => 100, 200 => 'def'],
    keyset[],
    keyset['a', 100, 'b'],
    [],
    [1, 2, 3],
    varray[],
    varray[1, 2, 3],
    darray[],
    darray[100 => 'abc', 'def' => 200]
  ];

  $tests = vec[
    'HH\\is_varray_or_darray',
    'gettype'
  ];

  foreach ($values as $v) { direct_test_dv_arrays($v); }
  foreach ($values as $v) { indirect_test_dv_arrays($v, $tests); }
}

function test2() {
  $values = vec[
    varray[],
    varray[1, 2, 3],
    darray[],
    darray[100 => 'abc', 'def' => 200]
  ];

  $tests = vec[
    'is_null',
    'is_bool',
    'is_int',
    'is_string',
    'is_float',
    'is_resource',
    'is_object',
    'is_array',
    'HH\\is_vec',
    'HH\\is_dict',
    'HH\\is_keyset',
    'is_scalar'
  ];

  foreach ($values as $v) { direct_test_others($v); }
  foreach ($values as $v) { indirect_test_others($v, $tests); }
}

test1();
test2();
