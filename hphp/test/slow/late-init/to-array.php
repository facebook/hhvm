<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public $x = 123;
  <<__LateInit>> public $y;
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

  $a = new A();
  try {
    $func($a);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $a->y = 700;
  try {
    $func($a);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  unset($a->y);
  try {
    $func($a);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
<<__EntryPoint>> function main(): void {
run_test('test_cast');
run_test('test_iter');
run_test('test_obj_prop_array');
}
