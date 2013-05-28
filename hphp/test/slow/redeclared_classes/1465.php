<?php

$i = 2;
if ($i == 1) {
  class foo {
    function foo() {
      echo "foo 1";
    }
    function bar() {
      echo "bar 1";
    }
  }
}
 else {
  class foo {
    function foo() {
      echo "foo 2";
    }
    function bar() {
      echo "bar 2";
    }
  }
}
$t = new foo();
$t->foo();
$t->bar();
