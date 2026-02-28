<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f(int $x) : void {
  // don't offer the refactoring: no indication that the user intends to refactor `$the_variable`
  $the_variable = 1 + /*range-start*/$x/*range-end*/;
}
