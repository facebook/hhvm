<?hh

function simple_for_each(vec<string> $x): string {
  $accum = "";
  foreach ($x as $val) {
    $accum .= $val;
  }
  return $accum;
}

function keyed_for_each(vec<string> $x): int {
  $accum = "";
  $last = 0;
  foreach ($x as $key => $val) {
    $accum .= $val;
    $last = $key;
  }
  return $last;
}
