<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function f(int $x, named int $n = 0): void {}

function multiple(
  int $x,
  named int $n = 0,
  int $y,
  named int $m = 0,
  int $z = 0,
): void {}

function variadic(int $x, named int $n = 0, int ...$rest): void {}

function test(): void {
  f(...tuple(0));
  f(...tuple(0), n = 0);
  f(n = 0, ...tuple(0));
  multiple(...tuple(0, 0));
  multiple(0, ...tuple(0));
  multiple(...tuple(0, 0, 0), n = 0, m = 0);
  variadic(...tuple(0));
  variadic(...tuple(0, 1, 2));
}
