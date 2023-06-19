<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(): void {
  let $x: arraykey = 1;
  if ($x is int) {
    hh_expect_equivalent<int>($x);
  } else {
    hh_expect_equivalent<nothing>($x);
  }
}

function g(): void {
  let $x: int = 1;
  if ($x is arraykey) {
    hh_expect_equivalent<int>($x);
  } else {
    hh_expect_equivalent<nothing>($x);
  }
}
