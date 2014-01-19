<?php

class c2 {
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
    c2::foo();
  }
}
$obj = new d2;
$obj->test1a();
