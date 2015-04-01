<?hh // strict

function n(): noreturn {
  throw new Exception('nope');
}

function v(): void {}

function test(): void {
  list($x, $y) = tuple(n(), v());
}
