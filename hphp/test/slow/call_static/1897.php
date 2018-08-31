<?php

class a2 {
  public function __call($func, $args) {
    var_dump('a2::__call');
  }
  public static function __callStatic($func, $args) {
    var_dump('a2::__callStatic');
  }
  public function test() {
    a2::foo();
  }
}
class b2 extends a2 {
  public function test() {
    a2::foo();
    b2::foo();
  }
}

<<__EntryPoint>>
function main_1897() {
$obj = new a2;
$obj->test();
$obj = new b2;
$obj->test();
}
