<?hh // strict

class X {
  <<__Deprecated('use bar() instead')>>
  public function foo(): void {}
}

function f(X $a): (function(X): void) {
  return inst_meth($a, 'foo');
}
