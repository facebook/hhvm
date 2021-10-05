<?hh
<<file:__EnableUnstableFeatures("readonly")>>
class Foo {}
function foo(
  inout readonly Foo $x // error
): void {
}
