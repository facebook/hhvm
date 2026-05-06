<?hh

function g(int $x): void {}

function f(mixed $x, bool $b): void {
  if ($b) {
    $x as int;
    g($x);
  }
  g($x);
}
