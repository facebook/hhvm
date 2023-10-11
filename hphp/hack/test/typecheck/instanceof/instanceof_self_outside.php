<?hh

function f(mixed $x): void {
  if ($x is self) {
    echo 'huh?';
  }
}
