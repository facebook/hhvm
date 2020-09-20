<?hh

class DeprecatedClass {
  <<__Deprecated('use bar() instead')>>
  public function foo(): void {}

  <<__Deprecated()>>
  public function bar(): void {}

  <<__Deprecated>>
  public function baz(): void {}

  <<__Deprecated('use bar2() instead', 5)>>
  public function foo2(): void {}
}

<<__Deprecated('use bar() instead')>>
function foo(): void {}

<<__Deprecated()>>
function bar(): void {}

<<__Deprecated>>
function baz(): void {}

<<__Deprecated('use bar2() instead', 5)>>
function foo2(): void {}
