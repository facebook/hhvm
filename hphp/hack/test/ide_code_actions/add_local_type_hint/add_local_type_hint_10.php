<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f(int $x) : void {
  $x = () ==> {
    // add a type hint for `$y`
    /*range-start*/$y = 3;/*range-end*/
  }();
}
