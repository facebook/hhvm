<?hh

function test($name, $c) :mixed{
  echo "---- $name\n";
  foreach ($c->keys() as $k) {
    var_dump($k);
  }
}

function main() :mixed{
  $containers = dict[
    'Vector'          => Vector {1},
    'empty Vector'    => Vector {},
    'ImmVector'       => ImmVector {2},
    'empty ImmVector' => ImmVector {},

    'Map'             => Map {3 => 4},
    'empty Map'       => Map {},
    'ImmMap'          => ImmMap {5 => 6},
    'empty ImmMap'    => ImmMap {},

    'Set'             => Set {7},
    'empty Set'       => Set {},
    'ImmSet'          => ImmSet {8},
    'empty ImmSet'    => ImmSet {},
  ];
  foreach ($containers as $name => $c) {
    test($name, $c);
  }
}


<<__EntryPoint>>
function main_keys() :mixed{
main();
}
