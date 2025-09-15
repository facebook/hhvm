<?hh
// We had some probably-unreachable parser logic for
// "double-variadic-expressions" in argument lists.
// Show what currently happens (not accepted).

function f(mixed ...$args): void {}

function test_nested_argument_splats(): void {
  $v = vec[];

  f(...$v);
  f(......$v);
  f(... ...$v);
  f(...(...$v));
}
