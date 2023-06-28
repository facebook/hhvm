<?hh

// Test materialization to & from ImmVector.

// Prints the entries of a Vector in sorted order.
// We need this because some collections are not ordered.
function printNormalized(ImmVector $fv) :mixed{
  $v = new Vector($fv);
  sort(inout $v); // sorting not supported for FV yet...
  foreach ($v as $e) {
    var_dump($e);
  }
}

function testToImmVector() :mixed{
  printNormalized((Vector { 1, 2, 3 })->toImmVector());
  printNormalized((Map { 10 => "Hello", 20 => "World" })->toImmVector());

  printNormalized((Set { 10, 20, 30 })->toImmVector());
  printNormalized((Pair {1, 2})->toImmVector());
}

function testFromImmVector() :mixed{
  $fv = ImmVector {1, 2, 3};
  var_dump($fv->toVector() == Vector {1, 2, 3});
  printNormalized($fv->toImmVector());
  var_dump($fv->toMap() == Map { 0 => 1, 1 => 2, 2 => 3 });
  var_dump($fv->toSet() == Set { 1, 2, 3 });
}

function main() :mixed{
  testToImmVector();
  echo ".........\n";
  testFromImmVector();
}


<<__EntryPoint>>
function main_materialization() :mixed{
main();
}
