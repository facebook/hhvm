<?hh

function test($name, $a, $b) :mixed{
  echo "---- $name\n";
  var_dump($a->concat($b));
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
  foreach ($containers as $name => $container) {
    foreach ($containers as $name2 => $container2) {
      test("$name to $name2", $container, $container2);
    }
  }
}


<<__EntryPoint>>
function main_concat_empty() :mixed{
main();
}
