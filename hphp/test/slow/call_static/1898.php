<?php

class a1 {
  public static function __callStatic($func, $args) {
    var_dump('a1::__callStatic');
  }
  public function test() {
    a1::foo();
  }
}
class b1 {
  public function test() {
    a1::foo();
  }
}
$obj = new a1;
$obj->test();
$obj = new b1;
$obj->test();
