<?hh // strict

function f(?string $x, int $y): void {
  if ($y) {
  }
  $z = $x ?: 'bar';
}
