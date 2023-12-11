<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a, $b) :mixed{
  echo "====================================================\n";
  var_dump($a);
  var_dump($b);
  $x = array_merge($a, $b);
  var_dump($x);
  var_dump(HH\is_php_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
}

function test_all() :mixed{
  $vals = vec[
    vec[],
    vec['a', 'b', 'c'],
    dict[],
    dict[10 => 'a', 20 => 'b', 30 => 'c'],
    dict['a' => 123, 'b' => 456, 'c' => 789],
    vec[],
    vec['a', 'b', 'c'],
    dict[],
    dict[10 => 'a', 20 => 'b', 30 => 'c'],
    dict['a' => 123, 'b' => 456, 'c' => 789]
  ];

  foreach ($vals as $v1) {
    foreach ($vals as $v2) {
      test($v1, $v2);
    }
  }
}

<<__EntryPoint>>
function main_array_merge() :mixed{
test_all();
}
