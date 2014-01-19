<?php

trait T1 {
  abstract function bar();
  public function foo() {
    $this->bar();
  }
}

trait T2 {
  public function bar() {
    echo "Hello from bar()\n";
  }
}

class C {
  use T1, T2;
}

$o = new C;
$o->foo();


