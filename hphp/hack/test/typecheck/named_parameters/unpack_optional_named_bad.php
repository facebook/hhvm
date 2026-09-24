<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function f(int $x, int $y, named int $n = 0): void {}

function required_named(int $x, named int $n, named int $m = 0): void {}

function test(): void {
  f(...tuple(0));
  f(...tuple(0, 0, 0));
  f(...tuple('wrong', 0));
  f(...tuple(0, 0), n = 'wrong');
  required_named(...tuple(0));
}
//
