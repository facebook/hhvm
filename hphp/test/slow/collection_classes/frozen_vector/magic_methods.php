<?hh

// Test magic methods for ImmVector.

// Call $f(); if it throws, return the exception's error message.
// Otherwise, return $f's return value.
function callFailsafe((function (ImmVector): string) $f) {
  $fv = ImmVector {1, 2, 3};

  try {
    $r = $f($fv);
  } catch (Exception $e) {
    return $e->getMessage();
  }

  return $r;
}

function main() {

  $functions = ImmVector {
    function ($fv) {
      // __toString()
      return (string)$fv;
    },
    function ($fv) {
      // __get()
      $p = $fv->nonExistentProperty; // should throw
      return "SomethingWentWrong";
    },
    function ($fv) {
      // __set()
      $fv->nonExistentProperty = 0; // should throw
      return "SomethingWentWrong";
    },
    function ($fv) {
      // __isset()
      return isset($v->nonExistentProperty) ? "true" : "false";
    },
    function ($fv) {
      // __unset()
      unset($fv->nonExistentProperty); // should throw
      return "SomethingWentWrong";
    }
  };

  foreach ($functions as $f) {
    echo callFailsafe($f) . "\n";
  }
}

main();
