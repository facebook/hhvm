<?php

class Foo {
  private function fun() {
    return "fun";
  }
  function __call($x, $y) {
    return "__call";
  }

  function go() {
    $y = Foo::fun();
    var_dump($y);
    $y = Foo::fun();
    var_dump($y);
  }
}

function main() {
  (new Foo)->go();
}


<<__EntryPoint>>
function main_magic_call_008() {
main();
}
