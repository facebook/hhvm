<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

interface I {}
;

interface J extends I {}
;

function f(int $i, J $j): void {
  let $x: arraykey = 1;
  switch ($i) {
    case 0:
      let $x: int = 1;
      let $y: I = $j;
      break;
    default:
      let $y: J = $j;
      break;
  }
  $x = 4;
  $y = $j;
}
