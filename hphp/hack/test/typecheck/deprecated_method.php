<?hh

class F {
  <<__Deprecated('use bar() instead')>>
  public static function foo() {}
}

function f() {
  F::foo();
}
