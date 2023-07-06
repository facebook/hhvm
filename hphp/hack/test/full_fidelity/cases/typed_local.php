<?hh

<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  let $x : int = 1;

  let int $x = 1;
  let $x : int : 1;
  let $x: int;
  let $x int;
}
