<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>
<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

interface I {}
;

interface J extends I {}
;

function f(bool $b, J $j): void {
  let $x: arraykey = 1;
  if ($b) {
    let $x: int = 1;
    let $y: I = $j;
  } else {
    let $y: J = $j;
  }
  $x = 4;
  $y = $j;
}

function g(bool $b): void {
  /* HH_FIXME[4110] silence error when control flow joins at the end of the if
    * because we're just trying to test the type of $x after a join error. */
  if ($b) {
    let $x: arraykey = "";
  } else {
    let $x: num = 1.0;
  }
  hh_expect_equivalent<int>($x);
}
