<?hh

function err($x, $y) { throw new Exception('heh'); }

function foo() {
  echo "----\n";
  $lol = new stdClass;
  $x = darray[];
  try {
    $x[$lol] = 2;
  } catch (Exception $y) {
    echo "after a throw:\n";
    set_error_handler(null);
    var_dump($y->getMessage());
    var_dump($x);
    return;
  }
  var_dump($x);
}


<<__EntryPoint>>
function main_promote_exn_001() {
foo();
set_error_handler(err<>);
foo();
}
