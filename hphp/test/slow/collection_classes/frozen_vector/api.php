<?hh

// Test miscellaneous methods of the ImmVector API.

function main() {

  $fv = ImmVector {1, 2, 3};

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
}

main();
