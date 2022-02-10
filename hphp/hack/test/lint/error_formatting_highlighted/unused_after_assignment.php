<?hh

function unused_after_assignment($t) {
  $x = 1 + 2;
  $y = 'a' . 'b';
  return $t + $x;
}
