<?hh // strict

class Foo {}

function test(bool $b, Foo $f1, ?Foo $f2): void {
  $x = $b ? $f1 : $f2;
  // Not an error--eliminating nullability from (Foo | ?Foo) is reasonable.
  if ($x) {
  }
}
