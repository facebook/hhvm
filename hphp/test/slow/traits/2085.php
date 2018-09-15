<?php

trait Too {
  function bar() {
    $abc = 123;
    $a = function ($x) use ($abc) {
      $n = func_num_args();
      $args = func_get_args();
      var_dump($n, $args);
    }
;
    return $a;
  }

  function baz($obj) {
    $abc = 456;
    $obj(789);
  }
}
class Foo {
 use Too;
 }

<<__EntryPoint>>
function main_2085() {
$a = Foo::bar();
Foo::baz($a);
}
