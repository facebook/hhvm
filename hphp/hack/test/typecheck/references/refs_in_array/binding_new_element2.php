<?hh // strict

function test(): void {
  $y = 42;
  $x = array(1);
  $x[1] = &$y;
}
