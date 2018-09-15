<?php

class a1 {
  public function __call($func, $args) {
    var_dump('a1::__call');
  }
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

<<__EntryPoint>>
function main_1896() {
$obj = new a1;
$obj->test();
$obj = new b1;
$obj->test();
}
