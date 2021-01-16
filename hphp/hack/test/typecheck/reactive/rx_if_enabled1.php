<?hh // strict
<<__Rx>>
function f1(): int {
  if (Rx\IS_ENABLED) {
    return 1;
  } else {
    print 'non-rx';
    return 2;
  }
}
