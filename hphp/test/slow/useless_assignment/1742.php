<?php

class A {
  function __destruct() {
    var_dump('done');
  }
}
function foo() {
  $a = 10;
  if ($a == 11) {
    return null;
  }
  return new A();
}
function bar() {
  $a = foo();
  var_dump('doing');
}

<<__EntryPoint>>
function main_1742() {
bar();
}
