<?hh // strict

class Foo {
  public static function x(): void {}
}

class Bar extends Foo {}

enum E: classname<Foo> as classname<Foo> {
  FOO = Foo::class;
  Bar = Bar::class;
}

function f(): void {
  $cls = E::FOO;
  $cls::x();
}
