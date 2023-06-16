<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  // should offer no refactoring
  /*range-start*/let list($x, $y) = tuple(1, 2);/*range-end*/
}
