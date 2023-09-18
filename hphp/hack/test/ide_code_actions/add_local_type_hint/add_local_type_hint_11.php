<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  // convert to `let $the_variable: vec<int> = vec[1];`
  /*range-start*/$the_variable/*range-end*/ = vec[1];
}
