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
  include '1860-1.inc';
} else {
  include '1860-2.inc';
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
