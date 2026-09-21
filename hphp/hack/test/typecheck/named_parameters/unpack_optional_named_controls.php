<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function positional(int $x, int $n = 0): void {}

function required_named(int $x, named int $n): void {}

function optional_named(int $x, named int $n = 0, int $y = 0): void {}

function variadic(int $x, named int $n = 0, int ...$rest): void {}

function test(): void {
  positional(...tuple(0));
  required_named(...tuple(0), n = 0);
  optional_named(0);
  optional_named(0, ...tuple(1));
  variadic(0, ...tuple(1, 2));
}
