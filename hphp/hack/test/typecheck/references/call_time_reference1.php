<?hh

function test($x, $y) {
  test($y, &$x);
}
