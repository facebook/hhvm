<?php

trait T {
  function foo() {
    echo "Foo";
    parent::bar();
    echo "__class__: " . __class__ . "\n";
  }
}
class C {
  function bar() {
    echo "BAR!\n";
  }
}
class D extends C {
  use T;
}
$x = new D();
$x->foo();
?>
