<?hh

function return_float(bool $x): float {
  /* HH_FIXME[4110] */
  return return_int($x);
}

function return_int(bool $x): int {
  if ($x) {
    return 1;
  } else {
    return 0;
  }
}
