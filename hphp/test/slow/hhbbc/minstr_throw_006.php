<?hh

<<__EntryPoint>>
function test() {
  try {
    $x->a = 2;
  } catch (Exception $e) {
    echo "catch\n";
    var_dump(isset($x));
  }
}
