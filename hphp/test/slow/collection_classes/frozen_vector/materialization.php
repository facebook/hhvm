<?hh

// Test materialization to & from FrozenVector.

// Prints the entries of a Vector in sorted order.
// We need this because some collections are not ordered.
function printNormalized(FrozenVector $fv) {
  $v = new Vector($fv);
  sort($v); // sorting not supported for FV yet...
  foreach ($v as $e) {
    var_dump($e);
  }
}

function testToFrozenVector() {
  printNormalized((Vector { 1, 2, 3 })->toFrozenVector());
  printNormalized((Map { 10 => "Hello", 20 => "World" })->toFrozenVector());

  printNormalized((StableMap { "first" => "A", "second" => "B" })
    ->toFrozenVector());

  printNormalized((Set { 10, 20, 30 })->toFrozenVector());
  printNormalized((Pair {1, 2})->toFrozenVector());
}

function testFromFrozenVector() {
  $fv = FrozenVector {1, 2, 3};
  var_dump($fv->toVector() == Vector {1, 2, 3});
  printNormalized($fv->toFrozenVector());
  var_dump($fv->toMap() == Map { 0 => 1, 1 => 2, 2 => 3 });
  var_dump($fv->toStableMap() == StableMap { 0 => 1, 1 => 2, 2 => 3 });
  var_dump($fv->toSet() == Set { 1, 2, 3 });
}

function main() {
  testToFrozenVector();
  echo ".........\n";
  testFromFrozenVector();
}

main();
