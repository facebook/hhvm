<?hh // strict

function expect_mixed_func((function(): mixed) $func): void {}

function test(): void {
  $func = (): int ==> 1;
  expect_mixed_func($func);
}
