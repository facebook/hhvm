<?php

class A {
  function __construct() {
    throw new Exception();
  }
}

class B {
}

function foo($a, $b) {
}

function bar($a, $b) {
}

bar(new B, foo(1, new A));
