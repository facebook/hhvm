<?hh

// Test magic methods for Pair.

// Call $f(); if it throws, return the exception's error message.
// Otherwise, return $f's return value.
function callFailsafe((function (Pair): string) $f) {
  $pp = Pair {1, 2};

  try {
    $r = $f($pp);
  } catch (Exception $e) {
    return get_class($e) . ': ' . $e->getMessage();
  }

  return $r;
}

function main() {

  $functions = FrozenVector {
    function ($pp) {
      // __toString()
      return (string)$pp;
    },
    function ($pp) {
      // __get()
      $p = $pp->nonExistentProperty; // should throw
      return "SomethingWentWrong";
    },
    function ($pp) {
      // __set()
      $pp->nonExistentProperty = 0; // should throw
      return "SomethingWentWrong";
    },
    function ($pp) {
      // __isset()
      return isset($v->nonExistentProperty) ? "true" : "false";
    },
    function ($pp) {
      // __unset()
      unset($pp->nonExistentProperty); // should throw
      return "SomethingWentWrong";
    }
  };

  foreach ($functions as $f) {
    echo callFailsafe($f) . "\n";
  }
}

main();
