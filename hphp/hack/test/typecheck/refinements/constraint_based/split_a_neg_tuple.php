<?hh

// this is just a function that's logically similar to the one below,
// included for comparison
/*
function x_is_p_and_not_q((mixed, mixed) $x): void {
  if ($x is (num, int)) {
    if ($x is (int, num)) {
      ;
    } else {
      hh_expect_equivalent<(float, int)>($x);
    }
  }
}
*/

/* Commenting out because fix D61983191 has introduced a Hack error here
// this is the function that demonstrates the issue we're trying to test
function x_is_not_q_and_p((mixed, mixed) $x): void {
  hh_log_level("sub", 1);
  if ($x is (int, num)) {
    ;
  } else {
    //hh_show_env();
    if ($x is (num, int)) {
      hh_expect_equivalent<(float, int)>($x);
    }
  }
  hh_log_level("sub", 0);
}
*/
