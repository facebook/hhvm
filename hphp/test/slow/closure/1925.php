<?php

class Foo {
  function bar() {
    $abc = 123;
    $a = function ($abc) use ($abc, $abc) {
      var_dump($abc);
    }
;
    return $a;
  }
}
$a = Foo::bar();
$a(456);
