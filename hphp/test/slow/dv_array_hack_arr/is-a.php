<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function boolstr($b) { return $b ? "TRUE" : "FALSE"; }

function direct_test_dv_arrays($v) {
  echo "============= direct_test_dv_arrays ================\n";
  echo "value => ";
  var_dump($v);
  $v2 = __hhvm_intrinsics\launder_value($v);
  echo "is_varray => " . boolstr(is_varray($v2)) . "\n";
  echo "is_darray => " . boolstr(is_darray($v2)) . "\n";
  echo "gettype => " . gettype($v2) . "\n";
}

function indirect_test_dv_arrays($v, $tests) {
  echo "============= indirect_test_dv_arrays ==============\n";
  echo "value => ";
  var_dump($v);
  $v2 = __hhvm_intrinsics\launder_value($v);
  foreach ($tests as $c) {
    echo "$c => " . (($c === "gettype")
                     ? __hhvm_intrinsics\launder_value($c)($v2)
                     : boolstr(__hhvm_intrinsics\launder_value($c)($v2)))
                  . "\n";
  }
}

function direct_test_others($v) {
  echo "============= direct_test_others ===================\n";
  echo "value => ";
  var_dump($v);
  $v2 = __hhvm_intrinsics\launder_value($v);
  echo "is_null => " . boolstr(is_null($v2)) . "\n";
  echo "is_bool => " . boolstr(is_bool($v2)) . "\n";
  echo "is_int => " . boolstr(is_int($v2)) . "\n";
  echo "is_string => " . boolstr(is_string($v2)) . "\n";
  echo "is_float => " . boolstr(is_float($v2)) . "\n";
  echo "is_resource => " . boolstr(is_resource($v2)) . "\n";
  echo "is_object => " . boolstr(is_object($v2)) . "\n";
  echo "HH\is_php_array => " . boolstr(HH\is_php_array($v2)) . "\n";
  echo "is_vec => " . boolstr(is_vec($v2)) . "\n";
  echo "is_dict => " . boolstr(is_dict($v2)) . "\n";
  echo "is_keyset => " . boolstr(is_keyset($v2)) . "\n";
  echo "is_scalar => " . boolstr(is_scalar($v2)) . "\n";
}

function indirect_test_others($v, $tests) {
  echo "============= indirect_test_others =================\n";
  echo "value => ";
  var_dump($v);
  $v2 = __hhvm_intrinsics\launder_value($v);
  foreach ($tests as $c) {
    $c2 = __hhvm_intrinsics\launder_value($c);
    echo "$c => " . boolstr($c2($v)) . "\n";
  }
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
    varray[],
    varray[1, 2, 3],
    darray[],
    darray[100 => 'abc', 'def' => 200],
    darray[0 => 'a', 1 => 'b', 2 => 'c'],
       varray[
      __hhvm_intrinsics\launder_value('x'),
      __hhvm_intrinsics\launder_value('y'),
      __hhvm_intrinsics\launder_value('z'),
    ],
    darray[
      0 => __hhvm_intrinsics\launder_value(123),
      1 => __hhvm_intrinsics\launder_value(456),
      2 => __hhvm_intrinsics\launder_value(789)
    ],
    darray[
      'x' => __hhvm_intrinsics\launder_value(123),
      'y' => __hhvm_intrinsics\launder_value(456),
      'z' => __hhvm_intrinsics\launder_value(789)
    ],
    darray[
      __hhvm_intrinsics\launder_value(66) => __hhvm_intrinsics\launder_value(11),
      __hhvm_intrinsics\launder_value(55) => __hhvm_intrinsics\launder_value(22),
      __hhvm_intrinsics\launder_value(44) => __hhvm_intrinsics\launder_value(33)
    ]
  ];

  $tests = vec[
    'HH\\is_varray',
    'HH\\is_darray',
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
    darray[100 => 'abc', 'def' => 200],
    darray[0 => 'a', 1 => 'b', 2 => 'c'],
    varray[
      __hhvm_intrinsics\launder_value('x'),
      __hhvm_intrinsics\launder_value('y'),
      __hhvm_intrinsics\launder_value('z'),
    ],
    darray[
      0 => __hhvm_intrinsics\launder_value(123),
      1 => __hhvm_intrinsics\launder_value(456),
      2 => __hhvm_intrinsics\launder_value(789)
    ],
    darray[
      'x' => __hhvm_intrinsics\launder_value(123),
      'y' => __hhvm_intrinsics\launder_value(456),
      'z' => __hhvm_intrinsics\launder_value(789)
    ],
    darray[
      __hhvm_intrinsics\launder_value(66) => __hhvm_intrinsics\launder_value(11),
      __hhvm_intrinsics\launder_value(55) => __hhvm_intrinsics\launder_value(22),
      __hhvm_intrinsics\launder_value(44) => __hhvm_intrinsics\launder_value(33)
    ]
  ];

  $tests = vec[
    'is_null',
    'is_bool',
    'is_int',
    'is_string',
    'is_float',
    'is_resource',
    'is_object',
    'HH\is_php_array',
    'HH\\is_vec',
    'HH\\is_dict',
    'HH\\is_keyset',
    'is_scalar'
  ];

  foreach ($values as $v) { direct_test_others($v); }
  foreach ($values as $v) { indirect_test_others($v, $tests); }
}

function test3() {
  echo "============= constant-folding ===================\n";
  var_dump(is_varray(123));
  var_dump(is_darray(123));

  var_dump(is_varray(vec['a', 'b', 'c']));
  var_dump(is_darray(vec['a', 'b', 'c']));

  var_dump(is_varray(dict[0 => 'a', 1 => 'b']));
  var_dump(is_darray(dict[0 => 'a', 1 => 'b']));

  var_dump(gettype(varray[]));
  var_dump(HH\is_php_array(varray[]));
  var_dump(is_vec(varray[]));
  var_dump(is_dict(varray[]));
  var_dump(is_varray(varray[]));
  var_dump(is_darray(varray[]));

  var_dump(gettype(darray[]));
  var_dump(HH\is_php_array(darray[]));
  var_dump(is_vec(darray[]));
  var_dump(is_dict(darray[]));
  var_dump(is_varray(darray[]));
  var_dump(is_darray(darray[]));

  var_dump(gettype(varray['a', 'b', 'c']));
  var_dump(HH\is_php_array(varray['a', 'b', 'c']));
  var_dump(is_vec(varray['a', 'b', 'c']));
  var_dump(is_dict(varray['a', 'b', 'c']));
  var_dump(is_varray(varray['a', 'b', 'c']));
  var_dump(is_darray(varray['a', 'b', 'c']));

  var_dump(gettype(darray[0 => 'a', 1 => 'b', 2 => 'c']));
  var_dump(HH\is_php_array(darray[0 => 'a', 1 => 'b', 2 => 'c']));
  var_dump(is_vec(darray[0 => 'a', 1 => 'b', 2 => 'c']));
  var_dump(is_dict(darray[0 => 'a', 1 => 'b', 2 => 'c']));
  var_dump(is_varray(darray[0 => 'a', 1 => 'b', 2 => 'c']));
  var_dump(is_darray(darray[0 => 'a', 1 => 'b', 2 => 'c']));
}


<<__EntryPoint>>
function main_is_a() {
test1();
test2();
test3();
}
