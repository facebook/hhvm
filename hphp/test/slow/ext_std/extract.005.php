<?php

class X {
  static function foo() {
    $t = "this";
    $$t = 5;
    extract(array("this" => "foo"));
    var_dump($this);
  }
}


<<__EntryPoint>>
function main_extract_005() {
(new X)->foo();
X::foo();
}
