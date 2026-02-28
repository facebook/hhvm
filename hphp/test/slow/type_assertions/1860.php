<?hh

class Base {
  public function f() :mixed{
    var_dump('Base::f');
  }
}
function get() :mixed{
 return true;
}
function f($x) :mixed{
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
<<__EntryPoint>>
function entrypoint_1860(): void {
  if (get()) {
    include '1860-1.inc';
  } else {
    include '1860-2.inc';
  }
  include '1860-after.inc';
  f(new Base);
  f(new X);
  f(new Y);
}
