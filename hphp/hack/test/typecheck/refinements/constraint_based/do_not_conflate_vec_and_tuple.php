<?hh

function myrand(): bool {
  return true;
}

function do_not_conflate_vec_and_tuple(): void {
  if (myrand()) {
    $x = 1;
  } else if (myrand()) {
    $x = vec[];
  } else {
    $x = tuple(0, 1);
  }
  if ($x is (int, int)) {
    return;
  } else {
    hh_expect<int>($x); // should error
  }
}
