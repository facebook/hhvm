<?hh

abstract class C {
  abstract const type T as arraykey;
  abstract public function f() : this::T;
}

function getaC(): C {
  throw new Exception();
}

function f() : int {
  $z = (() ==> getaC()->f())();
  // we want a nice error here
  return $z;
}
