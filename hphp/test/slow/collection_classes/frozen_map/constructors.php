<?hh

// Test constructing a FixedMap in different ways.
function show_keyed_iter($iter) {
  $vs = Vector {};

  foreach ($iter as $k => $v) {
    $vs[] = Pair {$k, $v};
  }

  usort($vs, function($p1, $p2) {
    if ($p1[0] < $p2[0]) return -1;
    else if ($p1[0] == $p2[0]) return 0;
    else return 1;
  });

  echo "...\n";
  foreach ($vs as $v) var_dump($v);
  echo "...\n";
}

function main() {
  echo "Vector\n";
  show_keyed_iter(new FixedMap(Vector {1, 2, 3}));
  echo "FixedVector\n";
  show_keyed_iter(new FixedMap(FixedVector {1, 2, 3}));
  echo "Map\n";
  show_keyed_iter(new FixedMap(Map {0 => 1, 10 => 2, 40 => 3}));
  echo "FixedMap\n";
  show_keyed_iter(new FixedMap(new FixedMap(Map {0 => 1, 10 => 2, 40 => 3})));
  echo "StableMap\n";
  show_keyed_iter(new FixedMap(StableMap {0 => 1, 10 => 2, 40 => 3}));
  echo "array\n";
  show_keyed_iter(new FixedMap(array(1, 2, 3)));

  // We can't construct a Map or FixedMap from a Set.
}

main();
