<?hh

// Test that Set::toArray() raises a warning on a int/string collision.

function should_warn() {
  var_dump((Set {1, 42, 13, '1', 'hello', 'world'})->toArray());
  var_dump((Set {0, '0'})->toArray());

  var_dump((Set {strval(PHP_INT_MAX), PHP_INT_MAX})->toArray());

  $minInt = -PHP_INT_MAX - 1;
  var_dump((Set {$minInt, strval($minInt)})->toArray());
}

function no_warn() {
  var_dump((Set {-0, '-0'})->toArray());
  var_dump((Set {13, '013'})->toArray());
}

function main() {
  echo "WARN\n";
  echo "-------------\n\n";
  should_warn();
  echo "\n";
  echo "DON'T WARN\n";
  echo "-------------\n\n";
  no_warn();
}

main();
