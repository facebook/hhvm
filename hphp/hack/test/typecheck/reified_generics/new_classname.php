<?hh

final class Foo<reify T>{}

function test(): void {
  $x = Foo::class;

  new $x();
}

function test2(classname<Foo<int>> $x): void {
  // There is no way to construct a classname to Foo<int>
  new $x();
}
