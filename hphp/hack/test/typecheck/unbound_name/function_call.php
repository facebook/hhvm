<?hh

function foo(mixed $x = null): void {}

function test(): void {
  foo();

  // Unbound name
  not_found();

  // The inner foo should be an unbound global constant
  foo(foo);
}
