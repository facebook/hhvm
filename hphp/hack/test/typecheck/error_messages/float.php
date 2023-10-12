<?hh // strict

function foo(): string {
  $x = 5 + 1.2;
  $x = 5 + $x;
  $x = $x + 5;
  $x = 5 + $x;
  $x = 5 + $x;
  return $x + 5;
}
