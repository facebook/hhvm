<?hh

final class Foo {
  private function bar(): void {}
}

function baz(): void {
    $foo = new Foo();
    $foo->bar();
}
