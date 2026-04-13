<?hh

class Foo {}

function test(mixed $x): void {
  $x as Foo;
}
