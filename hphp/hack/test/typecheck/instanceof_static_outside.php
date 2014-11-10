<?hh // strict

function f(mixed $x): void {
  if ($x instanceof static) {
    echo 'huh?';
  }
}
