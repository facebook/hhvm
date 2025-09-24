<?hh
<<file:__EnableUnstableFeatures('named_parameters_use')>>

function foo(named int $x): void {}

function main(): void {
  foo(x = 3);
  $x = vec[1,2,3];
  foo(...$x, x=3);
}
