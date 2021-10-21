<?hh
class Foo {}
function foo(
  inout Foo $x
): void {
  $x = readonly new Foo(); // error
}
