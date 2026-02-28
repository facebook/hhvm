<?hh


<<__EntryPoint>>
function main_1054() :mixed{
$x = new stdClass();
try {
  var_dump ($x == 1 && 1 == $x);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x == 1.0 && 1.0 == $x);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x > 0);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x >= 1);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x < 5);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
try {
  var_dump ($x <= 1);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
