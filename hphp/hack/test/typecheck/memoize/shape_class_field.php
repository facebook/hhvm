<?hh

class Bar implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

type MyShape = shape(
  'first' => ?float,
  'second' => ?Bar,
  'third' => ?string,
);

class Foo {
  <<__Memoize>>
  public function someMethod(MyShape $i): void {}
}

<<__Memoize>>
function some_function(MyShape $i): void {}
