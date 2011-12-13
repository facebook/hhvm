<?php

class A {
  public function foo() {
    print "A::foo\n";
    $this->bar();
  }
  protected function baz() {
    print "A::baz\n";
  }
  private function f() {
    echo "A::f\n";
  }
  public function g($a) {
    echo "A::g\n";
    $a->f();
  }
}

class B extends A {
  protected function bar() {
    print "B::bar\n";
    $this->baz();
  }
  public function h($a) {
    print "B::g\n";
    $a->f();
  }
}

$a = new A;
$b = new B();
$b->foo();
$b->g($a);
#$b->h($a);
