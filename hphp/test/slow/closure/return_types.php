<?hh

function good() :mixed{
  $x = 1;

  $f_hack = function (): int use ($x) {
    return $x + 1;
  };

  var_dump($f_hack());
}

function bad() :mixed{
  $x = 1;

  $f_hack = function (): string use ($x) {
    return $x + 1;
  };

  try {
    $f_hack();
    echo "Should have thrown!\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

}


<<__EntryPoint>>
function main_return_types() :mixed{
good();

set_error_handler(function ($no, $str) {
  throw new Exception($str);
});

bad();
}
