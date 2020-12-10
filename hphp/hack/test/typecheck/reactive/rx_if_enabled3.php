<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f1(bool $x): int {
  // ERROR
  if (HH\Rx\IS_ENABLED || $x) {
    return 1;
  } else {
    print 'non-rx';
  }
  return 5;
}
