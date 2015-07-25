<?hh // strict

abstract class Foo {
  abstract const type T;
}

interface I {
  abstract const type TFoo as Foo;

  public function foo(): this::TFoo::T;
}

abstract class C implements I {
  const type TFoo = Foo;
}
