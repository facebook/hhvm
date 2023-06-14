<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  let $x : int = 1;
  $x = 2;
  $x = "";
}
