<?hh

// Test that Map::toArray() raises a warning on a int/string collision.

function should_not_warn() {
  var_dump((Map {1 => 42, '1' => 13, 'hello' => 'world'})->toArray());
  var_dump((Map {0 => 'a', '0' => 'b'})->toArray());

  var_dump((Map {strval(PHP_INT_MAX) => 'a', PHP_INT_MAX => 'b'})->toArray());

  $minInt = -PHP_INT_MAX - 1;
  var_dump((Map {$minInt => 'a', strval($minInt) => 'b'})->toArray());
}

function no_warn() {
  var_dump((Map {-0 => 'a', '-0' => 'b'})->toArray());
  var_dump((Map {13 => 'a', '013' => 'b'})->toArray());
}

<<__EntryPoint>> function main(): void {
  echo "ALSO DON'T WARN\n";
  echo "-------------\n\n";
  should_not_warn();
  echo "\n";
  echo "DON'T WARN\n";
  echo "-------------\n\n";
  no_warn();
}
