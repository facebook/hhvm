<?hh


<<__EntryPoint>>
function main_1054() {
$x = new stdClass();
try {
  var_dump ($x == 1 && 1 == $x);
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x == 1.0 && 1.0 == $x);
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x > 0);
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x >= 1);
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x < 5);
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x <= 1);
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
}
