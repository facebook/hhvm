<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  $the_variable = 3;
  // do not offer the refactoring for uses, only definitions
  $x = /*range-start*/$the_variable/*range-end*/;
}
