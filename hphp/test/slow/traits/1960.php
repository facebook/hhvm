<?php

trait T {
  function foo() {
    echo "Foo";
    $this->bar();
  }
  abstract function bar();
}
class C {
  use T;
  function bar() {
    echo "BAR!\n";
  }
}
$x = new C();
$x->foo();
?>
