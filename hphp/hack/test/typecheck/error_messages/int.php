<?hh // strict

function foo(): string {
  $x = 5;
  $x = 5 + $x;
  $x = $x + 5;
  $x = 5 + $x;
  $x = 5 + $x;
  return 5 + $x;
}
