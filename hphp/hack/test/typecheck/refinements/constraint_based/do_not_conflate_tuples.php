<?hh

function do_not_conflate_tuples(mixed $x): void {
  if ($x is (int, int)) {
    return;
  } else if ($x is (string, string)) {
    hh_expect<nothing>($x); // should error
  }
}
