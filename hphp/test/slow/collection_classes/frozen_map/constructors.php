<?hh

// Test constructing a ImmMap in different ways.
function show_keyed_iter($iter) :mixed{
  $vs = Vector {};

  foreach ($iter as $k => $v) {
    $vs[] = Pair {$k, $v};
  }

  usort(inout $vs, function($p1, $p2) {
    if ($p1[0] < $p2[0]) return -1;
    else if ($p1[0] == $p2[0]) return 0;
    else return 1;
  });

  echo "...\n";
  foreach ($vs as $v) var_dump($v);
  echo "...\n";
}

function main() :mixed{
  echo "Vector\n";
  show_keyed_iter(new ImmMap(Vector {1, 2, 3}));
  echo "ImmVector\n";
  show_keyed_iter(new ImmMap(ImmVector {1, 2, 3}));
  echo "Map\n";
  show_keyed_iter(new ImmMap(Map {0 => 1, 10 => 2, 40 => 3}));
  echo "ImmMap\n";
  show_keyed_iter(new ImmMap(new ImmMap(Map {0 => 1, 10 => 2, 40 => 3})));
  echo "array\n";
  show_keyed_iter(new ImmMap(vec[1, 2, 3]));

  // We can't construct a Map or ImmMap from a Set.
}


<<__EntryPoint>>
function main_constructors() :mixed{
main();
}
