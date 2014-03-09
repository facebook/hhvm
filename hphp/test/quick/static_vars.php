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

function init_twice($x) {
  if (!$x) {
    static $a = 42;
    $a--;
    print "init0 $a\n";
  } else {
    static $a = 47;
    $a++;
    print "init1 $a\n";
  }
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
init_twice(0);
init_twice(0);
init_twice(0);
init_twice(1);
init_twice(1);
init_twice(1);
