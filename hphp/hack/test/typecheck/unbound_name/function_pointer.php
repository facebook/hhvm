<?hh

function foo(): void {}

function test(): void {
  // No problem
  $x = foo<>;

  // Unbound name
  $y = not_found<>;
}
