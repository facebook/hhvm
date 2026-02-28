<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  // Do not offer the refactoring: hints are not valid here
  foreach(vec[1, 2] as /*range-start*/$the_variable/*range-end*/) {
    $the_variable = 1;
  }
}
