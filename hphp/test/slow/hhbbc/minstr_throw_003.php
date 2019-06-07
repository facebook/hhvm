<?hh

<<__EntryPoint>>
function test() {
  $x = array(array(array()));
  try {
    $x[0][0][0]->foo = 2;
  } catch (Exception $e) {
    var_dump(is_array($x));
    var_dump($x);
  }
}
