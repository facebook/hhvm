<?hh // partial

function test($x, $y) {
  test($y, &$x);
}
