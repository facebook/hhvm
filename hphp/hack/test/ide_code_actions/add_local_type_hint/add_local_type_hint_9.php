<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f(int $x) : void {
  // don't offer the refactoring when multiple statements are selected
  /*range-start*/
  $x = 0;
  $y = 0;
  /*range-end*/
}
