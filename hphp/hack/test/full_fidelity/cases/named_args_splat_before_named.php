<?hh
<<file:__EnableUnstableFeatures('named_parameters_use')>>

function main(): void {
  foo(...$tup, x = 5);
}
