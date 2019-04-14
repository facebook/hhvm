<?hh

abstract class C {
  abstract const type T;
  abstract public function get(): this::T;
}

function f(C $x): void {
  hh_show($x->get()); // <expr#1>::T
  function(C $x): void {
    hh_show($x->get()); // <expr#2>::T
  };
}
