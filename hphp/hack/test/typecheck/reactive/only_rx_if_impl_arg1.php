<?hh // strict

interface Rx {
  <<__Rx>>
  public function f(): int;
}

interface C {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f(): int;
  <<__Rx>>
  public function g(): int;
}

<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl(Rx::class)>>C $c): int {
  // OK: cond reactive function calls reactive method
  return $c->g();
}


<<__Rx, __AtMostRxAsArgs>>
function f1(<<__OnlyRxIfImpl(Rx::class)>>C $c): int {
  // OK: pass through argument and parameter has matching condition type
  return f($c);
}
