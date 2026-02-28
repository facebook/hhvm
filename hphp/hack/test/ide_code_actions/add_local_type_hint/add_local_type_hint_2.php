<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  // convert to `let $the_variable: int = 1;`
  /*range-start*/$the_variable = 1;/*range-end*/
}
