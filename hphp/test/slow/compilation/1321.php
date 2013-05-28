<?php

class X {
  function bar(X $x) {
    $x->foo();
    $x->foo();
  }
  function foo() {
 var_dump(__METHOD__);
 }
}
function test() {
  X::bar(null);
}
test();
