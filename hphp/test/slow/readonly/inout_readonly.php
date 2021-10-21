<?hh
class Foo {}
function foo(
  inout readonly Foo $x // error
): void {
}
