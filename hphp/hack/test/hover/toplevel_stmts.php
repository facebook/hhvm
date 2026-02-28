<?hh

$x = 3;

function get_num(): int {
  return 1;
}

$z = get_num() + $x;
  $z;
// ^ hover-at-caret
