<?hh
function foo(inout $a) { var_dump($a++); }
function test($cuf, $f) {
  $a = varray[1];
  try {
    $cuf($f, $a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, varray[1]);
  try {
    $cuf($f, varray[1]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, varray[1]);
  try {
    call_user_func_array($f, $a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, varray[1]);
  try {
    call_user_func_array($f, varray[1]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, varray[1]);
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);
  test('call_user_func_array', 'foo');
}
