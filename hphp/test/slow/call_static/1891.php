<?php

class b2 {
}
class c2 extends b2 {
  public function __call($func, $args) {
    echo "c2::__call
";
  }
  public function test1a() {
    b2::foo();
  }
}
set_error_handler('h');
function h() {
 var_dump('errored');
}
$obj = new c2;
$obj->test1a();
var_dump('end');
