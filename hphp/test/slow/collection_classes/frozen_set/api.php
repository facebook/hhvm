<?hh

// Test ImmSet's methods.

// Print every element of a ImmSet with a custom function,
// since serialization is not wired up yet.
function show_elems($fs) {
  echo "----\n";
  foreach ($fs as $e) {
    var_dump($e);
  }
  echo "----\n";
};

function api() {
  $fs = new ImmSet(Vector {1, 2, 3});
  $e = new ImmSet();

  echo "\nempty...\n";
  var_dump($fs->isEmpty());
  var_dump($e->isEmpty());

  echo "\ncount...\n";
  var_dump($fs->count());
  var_dump($e->count());

  echo "\nitems...\n";
  foreach ($fs->items() as $it) {
    var_dump($it);
  }

  foreach ($e->items() as $it) {
    var_dump($it);
  }

  echo "\nvalues...\n";
  var_dump($fs->values());
  var_dump($e->values());

  echo "\nlazy...\n";
  foreach ($fs->lazy() as $el) {
    var_dump($el);
  }

  echo "\ncontains...\n";
  var_dump($fs->contains(1));
  var_dump($fs->contains(10));
  var_dump($e->contains(0));
}

function materialization_methods() {
  $fs = new ImmSet(Vector {1, 2, 3});

  echo "\ntoArray...\n";
  var_dump($fs->toArray());

  echo "\ntoKeysArray...\n";
  var_dump($fs->toKeysArray());

  echo "\ntoValuesArray...\n";
  var_dump($fs->toValuesArray());

  echo "\ntoVector...\n";
  var_dump($fs->toVector());

  echo "\ntoImmVector...\n";
  var_dump($fs->toImmVector());

  echo "\ntoSet...\n";
  var_dump($fs->toSet());
}

function magic_methods() {
  echo "\n__toString...\n";
  echo (new ImmSet(Vector {1, 2, 3})) . "\n";

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
    $fs = new ImmSet(Vector {1, 2, 3});
    $x = $fs->nonexistentProperty;
  });

  echo "\n__set...\n";
  echo $cfail( function () {
    $fs = new ImmSet(Vector {1, 2, 3});
    $fs->inexistentProperty = 42;
  });

  echo "\n__isSet...\n";
  var_dump(isset((new ImmSet (Vector {1, 2, 3}))->notAProp));

  echo "\n__unset...\n";
  echo $cfail( function () {
    $fs = new ImmSet(Vector {1, 2, 3});
    unset($fs->inexistentProperty);
  });
}

function static_methods() {

  echo "\nfromItems...\n";
  show_elems(ImmSet::fromItems((Vector {1, 2, 3})->items()));
  show_elems(ImmSet::fromItems(Set {4, 5, 6}));

  echo "\nfromArrays...\n";
  show_elems(ImmSet::fromArrays(array(), array(1, 2, 3), array(4, 5, 6)));
}

function constructors() {
  echo "\nconstructors\n";
  show_elems(new ImmSet(Vector {1, 2, 3}));
  show_elems(new ImmSet(ImmVector {1, 2, 3}));
  show_elems(new ImmSet(Set {1, 2, 3}));
  show_elems(new ImmSet(Map {0 => 1, 10 => 2, 40 => 3}));
  show_elems(new ImmSet(array(1, 2, 3)));
}

function main() {
  api();
  materialization_methods();
  magic_methods();
  static_methods();
  constructors();
}

main();
