<?hh

function f(?int $i): ~int {
  hh_show($i);
  $j = HH\FIXME\UNSAFE_NONNULL_CAST($i);
  hh_show($j);
  return $j;
}
