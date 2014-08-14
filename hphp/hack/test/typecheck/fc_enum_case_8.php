<?hh

enum Bar : int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

// Non exhuastive match, but we don't care because no annotation
function do_case($x): int {
  switch ($x) {
    case Bar::FOO:
      return 0;
  }
}
