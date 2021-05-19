<?hh

function __return_3_tuple(bool $x): (int, int, int) {
  return tuple(1, 1, 1);
}

function __return_2_tuple(bool $x): (int, int) {
  return tuple(1, 1);
}

function assign_list_ok(bool $x): void {
  list($u, $v, $w) = __return_3_tuple($x);
}

function assign_list_bad(bool $x): void {
  /* HH_FIXME[4110] */
  list($u, $v, $w) = __return_2_tuple($x);
}
