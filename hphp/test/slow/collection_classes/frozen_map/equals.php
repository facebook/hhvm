<?hh

// Test equality of FixedMaps.
function main() {
  echo '= literal / variable equality =', "\n";
  $fm1 = FixedMap {};
  var_dump($fm1 == $fm1);
  var_dump($fm1 == FixedMap {});
  var_dump($fm1 == Map {});
  var_dump($fm1 == StableMap {});

  $fm2 = FixedMap {1 => 'a', 2 => 'b'};
  var_dump($fm2 == $fm2);
  var_dump($fm2 == FixedMap {1 => 'a', 2 => 'b',});
  var_dump(FixedMap {1 => 'a', 2 => 'b',} ==
           FixedMap {2 => 'b', 1 => 'a',});

  echo '= equality with other mapping types =', "\n";
  $fm = FixedMap {1 => 'a', 2 => 'b'};
  $m = Map {2 => 'b', 1 => 'a'};
  $sm = StableMap {2 => 'b', 1 => 'a'};
  var_dump($m == $fm);
  var_dump($fm == $m);
  var_dump($sm == $fm);
  var_dump($fm == $sm);

  echo '= inequality =', "\n";

  var_dump(FixedMap {1 => 'a', 2 => 'b'} == FixedMap {});
  var_dump(FixedMap {1 => 'a', 2 => 'b'} ==
           FixedMap {1 => 'b', 2 => 'a'});

  echo '= recursive equality =', "\n";

  var_dump(FixedMap {'a' => Vector {1}, 'b' => Vector {2}} ==
           FixedMap {'a' => Vector {1}, 'b' => Vector {3}});
  var_dump(FixedMap {'a' => Vector {1}, 'b' => Vector {2}} ==
           FixedMap {'a' => Vector {1}, 'c' => Vector {2}});

  $fm1 = FixedMap {'x' => FixedMap {1 => 'a', 2 => 'b'},
                    'y' => FixedMap {3 => 'c', 4 => 'd'}};
  $fm2 = FixedMap {'x' => FixedMap {1 => 'a', 2 => 'b'},
                    'y' => FixedMap {3 => 'c', 4 => 'd'}};
  var_dump($fm1 == $fm2);

  $fm1 = FixedMap {'a' => Vector {1}};
  $fm2 = FixedMap {0 => $fm1};
  $fm3 = FixedMap {0 => $fm2};

  $fm4 = FixedMap {0 => FixedMap {0 => FixedMap{'a' => Vector {1}}}};
  var_dump($fm3 == $fm4);
}

main();
