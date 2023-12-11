<?hh

function err($x, $y) :mixed{ throw new Exception('heh'); }

function foo() :mixed{
  echo "----\n";
  $lol = new stdClass;
  $x = dict[];
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
function main_promote_exn_001() :mixed{
foo();
set_error_handler(err<>);
foo();
}
