<?php

function foo() {
  static $a;
  $a++;
  print "foo $a\n";
}

function bar() {
  static $a;
  $a += 2;
  print "bar $a\n";
}

class A {
  private function priv() {
    static $x = 0;
    $x += 4;
    print "A::priv $x\n";
  }

  function baz() {
    static $a;
    $a += 4;
    print "A::baz $a\n";
    $this->priv();
  }
}

class B extends A {
}

$a = new A();
$b = new B();

foo();
bar();
$a->baz();
$b->baz();
foo();
bar();
$a->baz();
$b->baz();
