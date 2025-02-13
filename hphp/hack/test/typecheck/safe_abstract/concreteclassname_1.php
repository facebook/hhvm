<?hh

abstract class A {
  abstract public static function get(): int;
}
class C extends A {
  public static function get(): int {
    return 42;
  }
}

function expect_concrete(concreteclassname<A> $an): void {}

function expect_classname(classname<A> $an): void {}

<<__EntryPoint>>
function main(): void {
  $an = A::class;
  $cn = C::class;
  expect_classname($an); // ok
  expect_classname($cn); // ok
  expect_concrete($cn); // ok
  expect_concrete($an); // error
}
