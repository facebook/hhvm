<?hh

class X {
  <<__Deprecated('use bar() instead')>>
  public function foo(): void {}
}

function test(): (function(X): void) {
  return meth_caller('X', 'foo');
}
