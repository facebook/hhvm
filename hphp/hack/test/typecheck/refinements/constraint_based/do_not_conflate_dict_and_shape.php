<?hh

function myrand(): bool {
  return true;
}

function do_not_conflate_dict_and_shape(): void {
  if (myrand()) {
    $x = 1;
  } else if (myrand()) {
    $x = dict[];
  } else {
    $x = shape('x' => 0);
  }
  if ($x is shape('x' => int)) {
    return;
  } else {
    hh_expect<int>($x); // should error
  }
}
