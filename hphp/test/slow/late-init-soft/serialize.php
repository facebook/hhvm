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

class B {
  public $x = 123;
  <<__SoftLateInit>> public $y;
  public $z = 'abc';
  public function __sleep() {
    return ['x', 'y'];
  }
}

class C {
  public $x = 123;
  <<__SoftLateInit>> public $y;
  public $z = 'abc';
  public function __sleep() {
    return ['x', 'z'];
  }
}

function serialize_test($a) {
  var_dump(serialize($a));
}

function json_encode_test($a) {
  var_dump(json_encode($a));
}

function var_dump_test($a) {
  var_dump($a);
}

function var_export_test($a) {
  var_export($a);
  echo "\n";
}

function print_r_test($a) {
  print_r($a);
}

function test($cls, $func) {
  echo "=============== $func ================\n";

  HH\set_soft_late_init_default(123);

  $a = new $cls();
  $func($a);
  HH\set_soft_late_init_default(456);
  $func($a);

  $a = new A();
  $a->y = 777;
  $func($a);

  unset($a->y);
  $func($a);
}

function run_serialize_tests($cls) {
  echo "===================== run_serialize_tests ====================\n";
  test($cls, 'serialize_test');
  test($cls, 'json_encode_test');
  test($cls, 'var_dump_test');
  test($cls, 'var_export_test');
  test($cls, 'print_r_test');
}

function unserialize_test($s) {
  echo "================= unserialize_test ======================\n";
  $a = unserialize($s);
  var_dump($a->y);
}

function main() {
  set_error_handler('error_handler');

  run_serialize_tests('A');
  run_serialize_tests('B');
  run_serialize_tests('C');

  HH\set_soft_late_init_default(vec[]);
  unserialize_test('O:1:"A":3:{s:1:"x";i:123;s:1:"y";i:777;s:1:"z";s:3:"abc";}');
  unserialize_test('O:1:"A":2:{s:1:"x";i:123;s:1:"z";s:3:"abc";}');
}

main();
