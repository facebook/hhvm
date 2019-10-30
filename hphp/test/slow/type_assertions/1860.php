<?hh

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
  if ($x is Base) {
    $x->f();
  }
  if ($x is X) {
    $x->f();
  }
  if ($x is Y) {
    $x->f();
  }
}
f(new Base);
f(new X);
f(new Y);
