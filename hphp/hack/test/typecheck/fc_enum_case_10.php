//// def.php
<?hh // strict

enum Bar : int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

newtype Foo = Bar;

class Welp {
  const Foo FOO = Bar::FOO;
}

//// use.php
<?hh // strict

// Case not exhaustive, but we're casing on a newtype, so it is fine.
// We *couldn't* case on the whole thing if we wanted.
function do_case(Foo $x): int {
  switch ($x) {
    case Welp::FOO:
      return 0;
  }
}
