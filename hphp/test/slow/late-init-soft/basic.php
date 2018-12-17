<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function error_handler($errno, $errstr) {
  echo $errstr . "\n";
}

class A {
  <<__SoftLateInit>> public $a;
  <<__SoftLateInit>> private $b;

  public function get() {
    return $this->b;
  }

  public function set($x) {
    $this->b = $x;
  }
}

function test($a) {
  var_dump($a->a);
  var_dump($a->get());

  var_dump($a->a);
  var_dump($a->get());
}

function main() {
  set_error_handler('error_handler');

  test(new A());

  HH\set_soft_late_init_default(123);
  test(new A());

  HH\set_soft_late_init_default('abc');
  test(new A());

  $a = new A();
  test($a);
  HH\set_soft_late_init_default('def');
  test($a);

  $a = new A();
  $a->a = 789;
  $a->set(789);
  test($a);
}

main();
