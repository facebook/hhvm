<?hh // strict

function f(bool $b, int $x): void {
  $y = null;
  if ($b) {
    $y = $x;
  }
  if ($y) {
  }
}
