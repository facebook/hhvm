<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

interface I {}
;

interface J extends I {}
;

function f(int $e, J $j, I $i): void {
  let $x: arraykey = 1;
  switch ($e) {
    case 0:
      let $x: int = 1;
      let $y: I = $j;
      break;
    default:
      let $y: J = $j;
      let $z: int = 1;
      break;
  }
  $x = ""; // error, $x has bound int
  $y = $i; // error, $y has bound J
  $z; // error, $z is not defined
  $z = ""; // error, $z has bound int
}
