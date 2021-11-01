<?hh

function g(int $x): void {}

function f(mixed $x, bool $b): void {
  if ($x as @int) {
    g($x);
  }
}
