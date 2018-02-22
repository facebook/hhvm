<?hh // strict

function test(): void {
  $y = 42;
  $x = array();
  $x[] = &$y;
}
