<?hh

// Test that ImmMap correctly handled magic methods.

function main() {
  echo "\n__toString...\n";
  echo (new ImmMap(Vector {1, 2, 3})) . "\n";

  $cfail = function($f) {
    try {
      $f();
    } catch (Exception $e) {
      return $e->getMessage() . "\n";
    }
    return "WrongOutput\n";
  };

  echo "\n__get...\n";
  echo $cfail( function () {
  $fm = new ImmMap(Vector {1, 2, 3});
    $x = $fm->nonexistentProperty;
  });

  echo "\n__set...\n";
  echo $cfail( function () {
    $fm = new ImmMap(Vector {1, 2, 3});
    $fm->inexistentProperty = 42;
  });

  echo "\n__isSet...\n";
  var_dump(isset((new ImmMap (Vector {1, 2, 3}))->notAProp));

  echo "\n__unset...\n";
  echo $cfail( function () {
    $fm = new ImmMap(Vector {1, 2, 3});
    unset($fm->inexistentProperty);
  });
}

main();
