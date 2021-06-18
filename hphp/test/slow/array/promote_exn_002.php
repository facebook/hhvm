<?hh

function err($x, $y) { throw new Exception('heh'); }

class bar {}

function foo() {
  $bar = new bar;
  echo "----\n";
  $lol = new stdClass;
  $bar->x = darray[];
  try {
    $bar->x[$lol] = 2;
  } catch (Exception $y) {
    echo "after a throw:\n";
    set_error_handler(null);
    var_dump($y->getMessage());
    var_dump($bar);
    return;
  }
  var_dump($bar);
}


<<__EntryPoint>>
function main_promote_exn_002() {
foo();
set_error_handler(err<>);
foo();
}
