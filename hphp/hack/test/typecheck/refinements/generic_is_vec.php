<?hh

function takes_vec(vec<mixed> $x): void {}

function foo<T>(T $x): void {
  if ($x is vec<_>) {
    takes_vec($x);
  }
}
