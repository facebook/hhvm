<?hh // strict

function f(mixed $x): void {
  if ($x instanceof self) {
    echo 'huh?';
  }
}
