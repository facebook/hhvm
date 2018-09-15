<?php

function foo($a) {
  if ($a) {
    class A {
}
  }
}
function bar() {
  if (class_exists('A')) {
    class C extends A {
 }
    $obj = new C;
    var_dump($obj);
  }
 else {
    var_dump('no');
  }
}

<<__EntryPoint>>
function main_1224() {
foo(true);
bar();
}
