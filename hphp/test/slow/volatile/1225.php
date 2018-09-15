<?php

function foo($a) {
  if ($a) {
    interface A {
}
  }
}
function bar() {
  if (interface_exists('A')) {
    class C implements A {
 }
    $obj = new C;
    var_dump($obj);
  }
 else {
    var_dump('no');
  }
}

<<__EntryPoint>>
function main_1225() {
foo(true);
bar();
}
