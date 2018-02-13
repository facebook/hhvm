<?hh

trait T {
  function C(inout $x) {}
}

class C {
  use T;
}
