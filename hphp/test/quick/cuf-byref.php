<?hh
error_reporting(-1);
function foo(&$a) { var_dump($a++); }
function test($cuf, $f) {
  $a = array(1);
  try {
    $cuf($f, $a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, array(1));
  try {
    $cuf($f, array(1));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, array(1));
  try {
    call_user_func_array($f, $a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, array(1));
  try {
    call_user_func_array($f, array(1));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, array(1));
}
test('call_user_func_array', 'foo');
