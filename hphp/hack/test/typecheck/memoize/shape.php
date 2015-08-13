<?hh // strict

type MyShape = shape(
  'first' => ?float,
  'second' => ?int,
  'third' => ?string,
);

class Foo {
  <<__Memoize>>
  public function someMethod(MyShape $i): void {}
}
