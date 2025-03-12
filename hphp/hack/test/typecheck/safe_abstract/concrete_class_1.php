<?hh
// THIS TEST IS ONLY MEANINGFUL WITH CLASS POINTERS ENABLED
abstract class A {
  abstract public static function get(): int;
}
class C extends A {
  public static function get(): int {
    return 42;
  }
}

function expect_concrete(concrete<class<A>> $an): void {}

function expect_classname(class<A> $an): void {}

<<__EntryPoint>>
function main(): void {
  $an = A::class;
  $cn = C::class;
  expect_classname($an); // ok when class pointers enabled
  expect_classname($cn); // ok when class pointers enabled
  expect_concrete($cn); // ok when class pointers enabled
  expect_concrete($an); // error
}
