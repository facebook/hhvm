<?php

trait TestTrait {
  public static function test() {
    return static::class;
  }
}

class A {
  use TestTrait;
}

class B extends A {}
echo B::test();

