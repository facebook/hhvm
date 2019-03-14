<?hh // partial

function foo(int $x): void {
  switch ($x) {
    case 1: // FALLTHROUGH
      3;
    case 2:
    case 3:
      break;

  }

}
