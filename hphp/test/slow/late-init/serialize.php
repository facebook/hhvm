<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public $x = 123;
  <<__LateInit>> public $y;
  public $z = 'abc';
}

class B {
  public $x = 123;
  <<__LateInit>> public $y;
  public $z = 'abc';
  public function __sleep() :mixed{
    return vec['x', 'y'];
  }
}

class C {
  public $x = 123;
  <<__LateInit>> public $y;
  public $z = 'abc';
  public function __sleep() :mixed{
    return vec['x', 'z'];
  }
}

function serialize_test($a) :mixed{
  var_dump(serialize($a));
}

function json_encode_test($a) :mixed{
  var_dump(json_encode($a));
}

function var_dump_test($a) :mixed{
  var_dump($a);
}

function var_export_test($a) :mixed{
  var_export($a);
  echo "\n";
}

function print_r_test($a) :mixed{
  print_r($a);
}

function test($a, $func) :mixed{
  echo "=============== $func ================\n";

  try {
    $func($a);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $a->y = 777;
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

function run_serialize_tests($a) :mixed{
  echo "===================== run_serialize_tests ====================\n";
  test($a, 'serialize_test');
  test($a, 'json_encode_test');
  test($a, 'var_dump_test');
  test($a, 'var_export_test');
  test($a, 'print_r_test');
}

function unserialize_test($s) :mixed{
  echo "================= unserialize_test ======================\n";
  $a = unserialize($s);
  try {
    var_dump($a->y);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
<<__EntryPoint>> function main(): void {
run_serialize_tests(new A());
run_serialize_tests(new B());
run_serialize_tests(new C());

unserialize_test('O:1:"A":3:{s:1:"x";i:123;s:1:"y";i:777;s:1:"z";s:3:"abc";}');
unserialize_test('O:1:"A":2:{s:1:"x";i:123;s:1:"z";s:3:"abc";}');
}
