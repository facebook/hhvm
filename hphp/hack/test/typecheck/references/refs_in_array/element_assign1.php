<?hh // strict

function test(): void {
  $y = 42;
  $z = &$y; // ERROR: no refs in strict mode
}
