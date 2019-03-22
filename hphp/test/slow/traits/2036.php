<?php

trait TestTrait {
  public static function test() {
    $fun = array('A', 'test');
    return 'Forwarded '.$fun();
  }
}

class A {
  public static function test() {
    return "Test A";
  }
}

class B extends A {
  use TestTrait;
}

echo B::test();

