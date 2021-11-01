<?hh

class F {
  <<__Deprecated('use bar() instead')>>
  public static function foo(): void {}
}

function f(): void {
  F::foo();
}
