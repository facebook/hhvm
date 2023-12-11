<?hh
function foo(inout $a) :mixed{ var_dump($a++); }
function test($cuf, $f) :mixed{
  $a = vec[1];
  try {
    $cuf($f, $a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, vec[1]);
  try {
    $cuf($f, vec[1]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, vec[1]);
  try {
    call_user_func_array($f, $a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, vec[1]);
  try {
    call_user_func_array($f, vec[1]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  var_dump($a, vec[1]);
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);
  test('call_user_func_array', foo<>);
}
