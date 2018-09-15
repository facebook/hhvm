<?php

class b2 {
 }
class c2 extends b2 {
  public static function __callStatic($func, $args) {
    echo "c2::__callStatic
";
 }
}
class d2 extends c2 {
  public function __call($func, $args) {
    echo "d2::__call
";
  }
  public function test1a() {
    b2::foo();
  }
}
function h() {
 var_dump('errored');
}

<<__EntryPoint>>
function main_1890() {
set_error_handler('h');
$obj = new d2;
$obj->test1a();
var_dump('end');
}
