<?hh

// Test miscellaneous methods of the ImmVector API.

function show_elems($fs) {
  echo '---- ', get_class($fs), ' ----', "\n";
  foreach ($fs as $e) {
    var_dump($e);
  }
  echo "----\n";
};

function materialization_methods() {
  $fv = ImmVector {1, 2, 3};

  echo "\nvalues...\n";
  var_dump($fv->values());

  echo "\ntoArray...\n";
  var_dump($fv->toArray());

  echo "\ntoKeysArray...\n";
  var_dump($fv->toKeysArray());

  echo "\ntoValuesArray...\n";
  var_dump($fv->toValuesArray());

  echo "\ntoSet...\n";
  var_dump($fv->toSet());

  echo "\ntoImmSet...\n";
  var_dump($fv->toImmSet());

  echo "\ntoVector...\n";
  var_dump($fv->toVector());
}

function search_methods() {
  $fv = ImmVector {1, 2, 3};
  echo "\nlinearSearch...\n";
  var_dump($fv->linearSearch(2));
  var_dump($fv->linearSearch(10));
}

function static_methods() {
  echo "\nfromItems...\n";
  show_elems(ImmVector::fromItems((Vector {1, 2, 3})->items()));
  show_elems(ImmVector::fromItems(Set {4, 5, 6}));

  echo "\nfromKeysOf...\n";
  show_elems(ImmVector::fromKeysOf(Vector {1, 2, 3}));
  show_elems(ImmVector::fromKeysOf(['a', 'b', 'c']));
  show_elems(ImmVector::fromKeysOf(Map {'a' => 1, 'b' => 2}));
  show_elems(ImmVector::fromKeysOf(['a' => 1, 'b' => 2]));
  show_elems(ImmVector::fromKeysOf(Set {4, 5, 6}));
}

function main() {
  materialization_methods();
  search_methods();
  static_methods();
}

main();
