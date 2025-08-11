<?hh
<<file:__EnableUnstableFeatures('named_parameters_use')>>

function main(): void {
  // TODO(named_params): We said we would support this but it currently errors
  foo(...$tup, x = 5);
}
