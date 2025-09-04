<?hh
<<file:__EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function foo(named int $x): void {}

function main(): void {
  foo(x=3);
}
