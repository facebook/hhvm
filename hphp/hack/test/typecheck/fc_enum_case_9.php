<?hh

enum Bar : int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

type Foo = Bar;

// We can make it through typedefs
function do_case(Foo $x): int {
  switch ($x) {
    case Bar::FOO:
      return 0;
  }
}
