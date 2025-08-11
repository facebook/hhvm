<?hh
<<file:__EnableUnstableFeatures('named_parameters_use')>>

function main(): void {
  test_named_args(1, the_arg_name = 2);
}
