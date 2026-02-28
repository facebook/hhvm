<?hh

// Test that ImmMap is immutable.

function wrap_exception(callable $f) :mixed{
  try {
    $f(ImmMap {'a' => 1, 'b' => 2, 'c' => 3});
  } catch (Exception $e) {
    return get_class($e) . ": " . $e->getMessage();
  }

  return "NO EXCEPTION -- WRONG :(";
}

function main() :mixed{
  // All of these should throw.
  $funcs = vec[
    function ($fm) {
      $fm['a'] = 1;
    },
    function ($fm) {
      $fm[] = 10;
    },
    function ($fm) {
      unset($fm['a']);
    },
  ];

  foreach ($funcs as $f) {
    echo wrap_exception($f) . "\n";
  }
}

function nomutatorfuncs() :mixed{
  $fm = ImmMap{};
  var_dump(method_exists($fm, 'add'));
  var_dump(method_exists($fm, 'addAll'));
  var_dump(method_exists($fm, 'set'));
  var_dump(method_exists($fm, 'setAll'));
  var_dump(method_exists($fm, 'remove'));
}


<<__EntryPoint>>
function main_immutability() :mixed{
main();
nomutatorfuncs();
}
