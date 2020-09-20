<?hh

function add($x, $y, $u) {
  var_dump((int)$u + ($x += $y));
}

function div($x, $y, $z) {
  try {
    var_dump((int)$z - ($x/$y));
  } catch (DivisionByZeroException $e) {
    echo $e->getMessage(), "\n";
  }
  try {
    var_dump((int)$z - ($x%$y));
  } catch (DivisionByZeroException $e) {
    echo $e->getMessage(), "\n";
  }
}


<<__EntryPoint>>
function main_edge_cases() {
add(PHP_INT_MAX, 5, -1000);
div(42.0, 0.0, 5);
}
