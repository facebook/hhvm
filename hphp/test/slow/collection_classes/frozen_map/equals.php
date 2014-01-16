<?hh

// Test equality of FrozenMaps.
function main() {
  echo '= literal / variable equality =', "\n";
  $fm1 = FrozenMap {};
  var_dump($fm1 == $fm1);
  var_dump($fm1 == FrozenMap {});
  var_dump($fm1 == Map {});
  var_dump($fm1 == StableMap {});

  $fm2 = FrozenMap {1 => 'a', 2 => 'b'};
  var_dump($fm2 == $fm2);
  var_dump($fm2 == FrozenMap {1 => 'a', 2 => 'b',});
  var_dump(FrozenMap {1 => 'a', 2 => 'b',} ==
           FrozenMap {2 => 'b', 1 => 'a',});

  echo '= equality with other mapping types =', "\n";
  $fm = FrozenMap {1 => 'a', 2 => 'b'};
  $m = Map {2 => 'b', 1 => 'a'};
  $sm = StableMap {2 => 'b', 1 => 'a'};
  var_dump($m == $fm);
  var_dump($fm == $m);
  var_dump($sm == $fm);
  var_dump($fm == $sm);

  echo '= inequality =', "\n";

  var_dump(FrozenMap {1 => 'a', 2 => 'b'} == FrozenMap {});
  var_dump(FrozenMap {1 => 'a', 2 => 'b'} ==
           FrozenMap {1 => 'b', 2 => 'a'});

  echo '= recursive equality =', "\n";

  var_dump(FrozenMap {'a' => Vector {1}, 'b' => Vector {2}} ==
           FrozenMap {'a' => Vector {1}, 'b' => Vector {3}});
  var_dump(FrozenMap {'a' => Vector {1}, 'b' => Vector {2}} ==
           FrozenMap {'a' => Vector {1}, 'c' => Vector {2}});

  $fm1 = FrozenMap {'x' => FrozenMap {1 => 'a', 2 => 'b'},
                    'y' => FrozenMap {3 => 'c', 4 => 'd'}};
  $fm2 = FrozenMap {'x' => FrozenMap {1 => 'a', 2 => 'b'},
                    'y' => FrozenMap {3 => 'c', 4 => 'd'}};
  var_dump($fm1 == $fm2);

  $fm1 = FrozenMap {'a' => Vector {1}};
  $fm2 = FrozenMap {0 => $fm1};
  $fm3 = FrozenMap {0 => $fm2};

  $fm4 = FrozenMap {0 => FrozenMap {0 => FrozenMap{'a' => Vector {1}}}};
  var_dump($fm3 == $fm4);
}

main();
