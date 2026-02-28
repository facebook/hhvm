<?hh

// Test miscellaneous methods of the ImmVector API.

function show_elems($fs) :mixed{
  echo '---- ', get_class($fs), ' ----', "\n";
  foreach ($fs as $e) {
    var_dump($e);
  }
  echo "----\n";
}
function materialization_methods() :mixed{
  $fv = ImmVector {1, 2, 3};

  echo "\nvalues...\n";
  var_dump($fv->values());

  echo "\ntoDArray...\n";
  var_dump($fv->toDArray());

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

function search_methods() :mixed{
  $fv = ImmVector {1, 2, 3};
  echo "\nlinearSearch...\n";
  var_dump($fv->linearSearch(2));
  var_dump($fv->linearSearch(10));
}

function static_methods() :mixed{
  echo "\nfromItems...\n";
  show_elems(ImmVector::fromItems((Vector {1, 2, 3})->items()));
  show_elems(ImmVector::fromItems(Set {4, 5, 6}));

  echo "\nfromKeysOf...\n";
  show_elems(ImmVector::fromKeysOf(Vector {1, 2, 3}));
  show_elems(ImmVector::fromKeysOf(vec['a', 'b', 'c']));
  show_elems(ImmVector::fromKeysOf(Map {'a' => 1, 'b' => 2}));
  show_elems(ImmVector::fromKeysOf(dict['a' => 1, 'b' => 2]));
  show_elems(ImmVector::fromKeysOf(Set {4, 5, 6}));
}

function main() :mixed{
  materialization_methods();
  search_methods();
  static_methods();
}

<<__EntryPoint>>
function main_api() :mixed{
;

main();
}
