<?hh // strict

enum Bar: int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

// Non exhuastive match
function do_case(Bar $x): int {
  switch ($x) {
    case Bar::FOO:
      return 0;
  }
}
