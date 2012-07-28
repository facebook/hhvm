<?php

class X {
  protected function foo() {
    echo "X::foo\n";
  }
  private function bar() {
    echo "X::bar\n";
  }
  private function baz() {
    echo "X::baz\n";
  }
}

class Y extends X {
  protected function bar() {
    echo "Y::bar\n";
  }
}

class A extends Y {
  protected function foo() {
    echo "A::foo\n";
  }
  protected function bar() {
    echo "A::bar\n";
  }
  protected function baz() {
    echo "A::bar\n";
  }
}

class B extends Y {
  function foo() {
    $a = new A;
    $a->foo();
    $a->bar();
#    $a->baz();
  }
}

$b = new B;
$b->foo();
