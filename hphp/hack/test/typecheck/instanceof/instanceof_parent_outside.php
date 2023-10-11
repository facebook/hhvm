<?hh

function f(mixed $x): void {
  if ($x is parent) {
    echo 'huh?';
  }
}
