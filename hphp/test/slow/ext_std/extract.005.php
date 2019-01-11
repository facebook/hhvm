<?php

class X {
  static function foo() {
    $t = "this";
    $$t = 5;
    $arr = array("this" => "foo");
    extract(&$arr);
    var_dump($this);
  }
}


<<__EntryPoint>>
function main_extract_005() {
(new X)->foo();
X::foo();
}
