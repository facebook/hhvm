<?hh

abstract class C {
  abstract const type T;
  abstract public function get(): this::T;
  abstract public function set(this::T $_): void;
}

function f(C $c): void {
  // <expr#1>::T will escape from the following
  // lambda, but it's harmless because the expression
  // it refers to ($c) existed before the lambda was
  // created
  $x = (() ==> $c->get());
  $x();
  // we don't want an error here
  $c->set($x());
}
