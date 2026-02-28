<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

interface I {}
;

interface J extends I {}
;

function f(bool $b, J $j, I $i): void {
  let $x: arraykey = 1;
  if ($b) {
    $x = 1;
    let $y: I = $j;
    $y = $i;
  } else {
    let $z: J = $j;
    $z = $j;
  }
  $x = 4;
}

function i(bool $b): void {
  if ($b) {
    let $x: int;
  }
  $x = 1;
}
