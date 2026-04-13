<?hh

class Foo {}

function test(mixed $x): void {
  $x is Foo;
}
