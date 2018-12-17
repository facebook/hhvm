<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function error_handler($errno, $errstr) {
  echo $errstr . "\n";
}

class A {
  public $x = 123;
  <<__SoftLateInit>> public $y;
  public $z = 'abc';
}

function test_cast($a) {
  var_dump((array)$a);
}

function test_iter($a) {
  foreach ($a as $p) {
    echo "$p ";
  }
  echo "\n";
}

function test_obj_prop_array($a) {
  var_dump(HH\object_prop_array($a));
}

function run_test($func) {
  echo "============= $func ===============\n";

  HH\set_soft_late_init_default(123);

  $a = new A();
  $func($a);
  HH\set_soft_late_init_default(456);
  $func($a);

  $a = new A();
  $a->y = 700;
  $func($a);

  unset($a->y);
  $func($a);
}

function main() {
  set_error_handler('error_handler');

  run_test('test_cast');
  run_test('test_iter');
  run_test('test_obj_prop_array');
}
main();
