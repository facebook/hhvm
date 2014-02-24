<?hh

// Test materialization to & from FixedVector.

// Prints the entries of a Vector in sorted order.
// We need this because some collections are not ordered.
function printNormalized(FixedVector $fv) {
  $v = new Vector($fv);
  sort($v); // sorting not supported for FV yet...
  foreach ($v as $e) {
    var_dump($e);
  }
}

function testToFixedVector() {
  printNormalized((Vector { 1, 2, 3 })->toFixedVector());
  printNormalized((Map { 10 => "Hello", 20 => "World" })->toFixedVector());

  printNormalized((StableMap { "first" => "A", "second" => "B" })
    ->toFixedVector());

  printNormalized((Set { 10, 20, 30 })->toFixedVector());
  printNormalized((Pair {1, 2})->toFixedVector());
}

function testFromFixedVector() {
  $fv = FixedVector {1, 2, 3};
  var_dump($fv->toVector() == Vector {1, 2, 3});
  printNormalized($fv->toFixedVector());
  var_dump($fv->toMap() == Map { 0 => 1, 1 => 2, 2 => 3 });
  var_dump($fv->toSet() == Set { 1, 2, 3 });
}

function main() {
  testToFixedVector();
  echo ".........\n";
  testFromFixedVector();
}

main();
