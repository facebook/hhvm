<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

function f() : void {
  let $x : int = 1;
  $x = 2;
  hh_expect_equivalent<int>($x);
  let $y : arraykey = 1;
  hh_expect_equivalent<int>($y);
  $y = "";
  hh_expect_equivalent<string>($y);
  $y = 1;
  hh_expect_equivalent<int>($y);
}
