<?hh

final class Foo {
  private int $bar = 1;
}

function baz(): void {
    $foo = new Foo();
    $foo->bar;
}
