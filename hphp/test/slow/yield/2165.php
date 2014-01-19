<?php

// redec class gen
function get() {
 return true;
 }
if (get()) {
  class X {

    public function yielder() {
 yield 'first';
 }
  }
}
 else {
  class X {

    public function yielder() {
 yield 'second';
 }
  }
}
$x = new X;
foreach ($x->yielder() as $foo) {
 var_dump($foo);
 }

// derive from redec gen
class Foo {
  public function fooMsg() {
 return 'foo';
 }
  public function fooGen() {
 yield $this->fooMsg();
 }
}
if (get()) {
  class Bar extends Foo {
    public function fooMsg() {
 return 'bar';
 }
    public function barMsg() {
 return 'bar';
 }
    public function barGen() {
 yield $this->barMsg();
 }
  }
}
 else {
  class Bar extends Foo {
}
}
$f = new Foo;
foreach ($f->fooGen() as $foo) {
 var_dump($foo);
 }
$b = new Bar;
foreach ($b->fooGen() as $foo) {
 var_dump($foo);
 }
foreach ($b->barGen() as $foo) {
 var_dump($foo);
 }

// conditional derive from redec gen
function get0() {
 return false;
 }
function f($x) {
  if ($x) {
    if (get0()) {
      class X1 {
        public function msg() {
 return 'first, first';
 }
        public function gen() {
 yield $this->msg();
 }
      }

    }
 else {
      class X1 {
        public function msg() {
 return 'first, second';
 }
        public function gen() {
 yield $this->msg();
 }
      }

    }

  }
 else {
    if (get()) {
      class Y extends X1 {
        public function msg() {
 return 'second, first';
 }
        public function gen() {
 yield $this->msg();
 }
      }

    }
 else {
      class Y extends X1 {
        public function msg() {
 return 'second, second';
 }
        public function gen() {
 yield $this->msg();
 }
      }

    }

  }
  $x = $x ? new X1 : new Y;
  foreach ($x->gen() as $foo) {
 var_dump($foo);
 }
}
f(true);
f(false);
