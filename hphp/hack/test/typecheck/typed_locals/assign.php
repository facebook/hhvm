<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  let $x: int = 1;
  $x = 2;
  hh_expect_equivalent<int>($x);
  let $y: arraykey = 1;
  hh_expect_equivalent<int>($y);
  $y = "";
  hh_expect_equivalent<string>($y);
  $y = 1;
  hh_expect_equivalent<int>($y);

  let $z: arraykey = 1;
  /* HH_FIXME[4110] */
  $z = null;
  hh_expect_equivalent<arraykey>($z); // $z reverts to bound after type error

  let $a: int;
  $a = 1;
}
