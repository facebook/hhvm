<?hh

final class C<T> {}

function foo<<<__Enforceable>> reify T>(T $x): void {}

function test(): void {
  $x = foo<int>; // Enforceable

  $y = foo<C<int>>; // Not enforceable
}
