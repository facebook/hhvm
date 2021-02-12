<?hh // strict

function f1(bool $x): int {
  // ERROR
  if (Rx\IS_ENABLED || $x) {
    return 1;
  } else {
    print 'non-rx';
  }
  return 5;
}
