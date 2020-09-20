<?hh

final class Foo {
  public function bar(mixed $_): void {}
}

function test(Foo $obj): void {
  // the inner bar should be an unbound constant name
  $obj->bar(bar);
}
