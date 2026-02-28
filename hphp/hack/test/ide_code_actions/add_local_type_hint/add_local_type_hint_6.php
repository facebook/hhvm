<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  $the_variable = 1;
  // do not offer the refactoring for compound assingments
  /*range-start*/$the_variable/*range-end*/ += 2;
}
