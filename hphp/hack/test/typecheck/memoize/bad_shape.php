<?hh

class Bar {}

type MyShape = shape(
  'first' => ?float,
  'second' => ?Bar,
  'third' => ?string,
);

class Foo {
  <<__Memoize>>
  public function someMethod(MyShape $i): void {}
}
