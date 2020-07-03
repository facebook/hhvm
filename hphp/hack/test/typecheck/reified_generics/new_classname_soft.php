<?hh

final class Foo<<<__Soft>> reify T>{}

function coerce(classname<Foo<int>> $x): void {}

function test(): void {
  $x = Foo::class;

  coerce($x);

  new $x();
}
