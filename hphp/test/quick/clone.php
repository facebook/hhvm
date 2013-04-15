<?php

class A {
  public $x;
  static $count = 0;

  public function __construct() {
    $this->x = ++self::$count;
  }

  public function __clone() {
    $this->x = ++self::$count;
  }
};

function main() {
  $a = new A;
  $a->y = "foo";
  $b = clone $a;
  $a->y = "bar";
  var_dump($b);


  class C {}
  $ten = 10;
  $d = new C();
  $d->thing = &$ten;
  unset($ten);  // now the property is the only reference

  var_dump($d);
  $e = clone $d;
  var_dump($d);  // the reference doesn't persist across the clone
}
main();

