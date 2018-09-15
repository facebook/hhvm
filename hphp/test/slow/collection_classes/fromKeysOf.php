<?hh

function show_elems($name, $fs) {
  echo "---- $name\n";
  echo get_class($fs) . "\n";
  foreach ($fs as $e) {
    var_dump($e);
  }
}
function set_from_keys($containers) {
  echo "Set::fromKeysOf...\n";
  foreach ($containers as $name => $c) {
    show_elems($name, Set::fromKeysOf($c));
  }
  echo "\n";
}

function vector_from_keys($containers) {
  echo "Vector::fromKeysOf...\n";
  foreach ($containers as $name => $c) {
    show_elems($name, Vector::fromKeysOf($c));
  }
  echo "\n";
}

function main() {
  $containers = array(
    'empty array'  => [],
    'packed array' => ['a', 'b', 'c'],
    'mixed array'  => ['a' => 1, 'b' => 2],

    'empty Vector'    => Vector {},
    'empty ImmVector' => ImmVector {},
    'empty Map'       => Map {},
    'empty ImmMap'    => ImmMap {},
    'empty Set'       => Set {},
    'empty ImmSet'    => ImmSet {},

    'Vector'    => Vector {1, 2, 3},
    'ImmVector' => ImmVector {1, 2, 3},
    'Map'       => Map {'a' => 1, 'b' => 2},
    'ImmMap'    => ImmMap {'a' => 1, 'b' => 2},
    'Set'       => Set {4, 5, 6},
    'ImmSet'    => ImmSet {4, 5, 6},
  );
  set_from_keys($containers);
  vector_from_keys($containers);
}

<<__EntryPoint>>
function main_from_keys_of() {
;
main();
}
