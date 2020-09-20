<?hh

final class Foo<reify T>{}

function test2(classname<Foo<nothing>> $x): void {
  new $x();
}

<<__EntryPoint>>
function my_main(): void {
  $x = Foo::class;
  test2($x);
}
