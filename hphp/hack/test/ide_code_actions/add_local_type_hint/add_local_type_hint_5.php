<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  $the_variable = 3;
  // don't offer the refactoring: not enough information to indicate the user intends to refactor `$x`
  $x = /*range-start*/$the_variable/*range-end*/;
}
