<?hh

abstract class A<+T> {}
final class B<+T> extends A<T> {} // covariant

function get_mixed(): mixed {
  return new B();
}

function get_a(): A<string> {
  return new B();
}

function f(): void {
  expect_b1(get_mixed() as B); // ok
  expect_b2(get_mixed() as B); // error

  expect_b1(get_mixed() as B<_>); // ok
  expect_b2(get_mixed() as B<_>); // error

  expect_b1(get_a() as B<_>); // ok
  expect_b2(get_a() as B<_>); // ok

}

function expect_b1(B<mixed> $b): void {}
function expect_b2(B<string> $b): void {}
