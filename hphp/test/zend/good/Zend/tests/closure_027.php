<?hh

function test(closure $a) {
  var_dump($a());
}

<<__EntryPoint>>
function main(): void {
  test(function() {
    return new stdClass;
  });

  test(function() {});

  try {
    $a = function($x) use ($y) {};
    test($a);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  test(new stdClass);
}
