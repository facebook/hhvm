<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

interface I {}
;

interface J extends I {}
;

function f(bool $b, J $j, I $i): void {
  let $x: arraykey = 1;
  switch ($b) {
    case true:
      $x = 1;
      let $y: I = $j;
      $y = $i;
      break;
    case false:
      let $z: J = $j;
      $z = $j;
  }
  $x = 4;
}

function i(bool $b): void {
  switch ($b) {
    case true:
      let $x: int;
      break;
  }
  $x = 1;
}
