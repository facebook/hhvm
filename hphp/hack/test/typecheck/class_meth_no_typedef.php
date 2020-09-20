<?hh // partial

class Foo {
  public static function f(): void {}
}

type Bar = Foo;

function f(): (function(): void) {
  return class_meth('Bar', 'f');
}
