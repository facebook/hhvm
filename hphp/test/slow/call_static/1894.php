<?php

class a1 {
  public function __call($func, $args) {
    var_dump('a1::__call');
  }
}
class b1 {
  public function test() {
    a1::foo();
  }
}
set_error_handler('h');
 function h() {
 var_dump('errored');
}
$obj = new b1;
$obj->test();
var_dump('end');
