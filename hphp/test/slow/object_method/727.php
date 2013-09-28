<?php

class foo {
  public function test1() {
    echo 'in test1';
  }
  public function test2() {
    self::test1();
    foo::test1();
    echo 'in test2';
  }
  public function test3() {
    echo 'in test3';
  }
  public static function test4() {
    echo 'in test4';
  }
}
$obj = new foo();
$obj->test2();
foo::test1();
foo::test2();
foo::test3();
$obj->test3();
$obj->test4();
