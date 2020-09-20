<?hh // partial

function f() {
  return 1;
}

function g(): void {
  $x = f();
  return $x;
}
