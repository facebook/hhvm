<?hh

// Test miscellaneous methods of the FrozenVector API.

function main() {

  $fv = FrozenVector {1, 2, 3};

  // values()
  var_dump($fv->values());

  // toarray()
  var_dump($fv->toArray());

  // tokeysarray()
  var_dump($fv->toKeysArray());

  // tovaluesarray()
  var_dump($fv->toValuesArray());

  // linearSearch()
  var_dump($fv->linearSearch(2));
  var_dump($fv->linearSearch(10));

  // slice
  var_dump(FrozenVector::slice($fv, 1, 2));
  var_dump(FrozenVector::slice($fv, 3, 0));
}

main();
