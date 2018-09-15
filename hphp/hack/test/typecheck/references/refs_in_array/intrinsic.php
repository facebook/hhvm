<?hh // strict

function test(): void {
  $y = 42;
  array(1, &$y);
}
