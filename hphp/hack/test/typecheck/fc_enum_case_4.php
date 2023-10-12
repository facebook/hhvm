<?hh // strict

// Make sure we block this trickiness:

enum Bar : int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

class Baz {
  const Bar BAZ = Bar::FOO;
}

// Redundant match
function do_case(Bar $x): int {
  switch ($x) {
    case Bar::FOO:
      return 0;
    case Bar::BAR:
      return 1;
    // Bogus!
    case Baz::BAZ:
      return 2;
  }
}
