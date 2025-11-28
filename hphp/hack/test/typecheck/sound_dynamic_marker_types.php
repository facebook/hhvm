<?hh

function f(HH\FIXME\POISON_MARKER<string> $poison): void {
  hh_show($poison);
}

function g(\HH_FIXME\MISSING_TYPE_IN_HIERARCHY $missing): int {
  hh_show($missing);
  // Check that this is unsound!
  return $missing;
}
