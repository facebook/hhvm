<?hh

enum Bar: int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

function do_case(Bar $x): int {
  switch ($x) {
    case Bar::FOO:
      return 0;
    case Bar::BAR:
      return 1;
    case Bar::BAZ:
      return 2;
  }
}
