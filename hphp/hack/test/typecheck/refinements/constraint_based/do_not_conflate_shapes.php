<?hh

function do_not_conflate_tuples(mixed $x): void {
  if ($x is shape('x' => int)) {
    return;
  } else if ($x is shape('x' => string)) {
    hh_expect<nothing>($x);  // should erro
  }
}
