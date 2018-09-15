<?hh // strict

<<__Rx>>
function f1(): int {
  $a = 5;
  // ERROR
  if (HH\Rx\IS_ENABLED) {
    return 1;
  } else {
    print 'non-rx';
    return 2;
  }
}
