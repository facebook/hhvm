<?hh // strict

function f(mixed $x): void {
  if ($x instanceof parent) {
    echo 'huh?';
  }
}
