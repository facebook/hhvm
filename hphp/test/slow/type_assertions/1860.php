<?php

class Base {
  public function f() {
    var_dump('Base::f');
  }
}
function get() {
 return true;
 }
if (get()) {
  class X {
    public function f() {
      var_dump('X1::f');
    }
  }
}
 else {
  class X {
    public function f() {
      var_dump('X2::f');
    }
  }
}
class Y extends X {
}
function f($x) {
  if ($x instanceof Base) {
    $x->f();
  }
  if ($x instanceof X) {
    $x->f();
  }
  if ($x instanceof Y) {
    $x->f();
  }
}
f(new Base);
f(new X);
f(new Y);
