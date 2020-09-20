<?hh // partial
class Foo {}

<<__Rx>>
function returns_foo_returner(): Rx<(function(Foo): Foo)> {
  return $foo ==> $foo;
}

<<__Rx>>
function my_test_function(<<__OwnedMutable>> Foo $foo): void {
  $x = returns_foo_returner();
  $new_foo = $x($foo);
}
