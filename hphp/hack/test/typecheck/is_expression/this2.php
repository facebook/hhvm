<?hh

function f(mixed $x): void {
  if ($x is this) {
    echo 'huh?';
  }
}
