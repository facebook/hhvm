<?hh // strict
<<__Rx>>
function f1(): int {
  // ERROR
  if (Rx\IS_ENABLED) {
    return 1;
  } else {
    print 'non-rx';
  }
  return 5;
}
