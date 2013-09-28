<?php

class c3 {
  public function __call($func, $args) {
    echo "c3::__call
";
  }
  public static function __callStatic($func, $args) {
    echo "c3::__callStatic
";
  }
  public function test1b() {
    c3::foo();
 // invokes c3::__callStatic
  }
}
class d3 extends c3 {
  public function test1b() {
    c3::foo();
 // invokes c3::__callStatic
  }
}

c3::test1b();
d3::test1b();
