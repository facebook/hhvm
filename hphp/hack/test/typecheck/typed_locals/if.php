<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>

interface I {};

interface J extends I {};

function f(bool $b, J $j) : void {
  let $x : arraykey = 1;
  if ($b) {
    let $x : int = 1;
    let $y : I = $j;
  } else {
    let $y : J = $j;
  }
  $x = 4;
  $y = $j;
}
