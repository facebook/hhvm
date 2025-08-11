<?hh

<<file:__EnableUnstableFeatures('named_parameters_use')>>
function main(): void {
  // not a named argument
  3 + x = 3;
  // not a named argument
  4 + (y = 3);
}
