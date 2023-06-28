<?hh

// Test that ImmVector is immutable.

function main() :mixed{

  $callWithExc = function ($f) {
    try {
      $f(ImmVector {1, 2, 3});
    } catch (Exception $e) {
      return get_class($e) . ": " . $e->getMessage();
    }

    return "NO EXCEPTION -- WRONG :(";
  };

  // All of these should throw.

  $fs = ImmVector {
    function ($fv) {
      $fv[0] = 1;
    },
    function ($fv) {
      $fv[] = 10;
    },
    function ($fv) {
      unset($fv[0]);
    }
  };

  foreach ($fs as $f) {
    echo $callWithExc($f) . "\n";
  }
}


<<__EntryPoint>>
function main_immutability() :mixed{
main();
}
