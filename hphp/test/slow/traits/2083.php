<?php

trait Too {
  function bar() {
    $abc = 123;
    $a = function ($abc) use ($abc, $abc) {
      var_dump($abc);
    }
;
    return $a;
  }
}
class Foo {
 use Too;
 }
$a = Foo::bar();
$a(456);
