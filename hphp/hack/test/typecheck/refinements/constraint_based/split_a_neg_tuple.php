<?hh

// this is just a function that's logically similar to the one below,
// included for comparison
function x_is_p_and_not_q((mixed, mixed) $x): void {
  if ($x is (num, int)) {
    if ($x is (int, num)) {
      ;
    } else {
      hh_expect_equivalent<(float, int)>($x);
    }
  }
}

// this is the function that demonstrates the issue we're trying to test
function x_is_not_q_and_p((mixed, mixed) $x): void {
  if ($x is (int, num)) {
    ;
  } else {
    if ($x is (num, int)) {
      hh_expect_equivalent<(float, int)>($x);
    }
  }
}
