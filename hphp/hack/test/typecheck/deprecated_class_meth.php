<?hh // strict

class X {
  <<__Deprecated('use bar() instead')>>
  public static function foo(): void {}
}

function f(): (function(): void) {
  return X::foo<>;
}
