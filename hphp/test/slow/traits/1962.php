<?php

class A {
  public static function who() {
    echo "A: " . __CLASS__;
  }
  use T;
}
trait T {
  public static function test() {
    static::who();
 // Here comes Late Static Bindings
  }
}
class B extends A {
  public static function who() {
    echo "B: " . __CLASS__;
  }
}
B::test();
?>

