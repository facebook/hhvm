<?hh

function f(nonnull $x): void {
  switch ($x) {
    case 42:
      break;
    case 'foo':
      break;
  }
}
