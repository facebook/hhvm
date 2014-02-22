<?hh

// Test equality of ImmMaps.
function main() {
  echo '= literal / variable equality =', "\n";
  $fm1 = ImmMap {};
  var_dump($fm1 == $fm1);
  var_dump($fm1 == ImmMap {});
  var_dump($fm1 == Map {});

  $fm2 = ImmMap {1 => 'a', 2 => 'b'};
  var_dump($fm2 == $fm2);
  var_dump($fm2 == ImmMap {1 => 'a', 2 => 'b',});
  var_dump(ImmMap {1 => 'a', 2 => 'b',} ==
           ImmMap {2 => 'b', 1 => 'a',});

  echo '= equality with other mapping types =', "\n";
  $fm = ImmMap {1 => 'a', 2 => 'b'};
  $m = Map {2 => 'b', 1 => 'a'};
  var_dump($m == $fm);
  var_dump($fm == $m);

  echo '= inequality =', "\n";

  var_dump(ImmMap {1 => 'a', 2 => 'b'} == ImmMap {});
  var_dump(ImmMap {1 => 'a', 2 => 'b'} ==
           ImmMap {1 => 'b', 2 => 'a'});

  echo '= recursive equality =', "\n";

  var_dump(ImmMap {'a' => Vector {1}, 'b' => Vector {2}} ==
           ImmMap {'a' => Vector {1}, 'b' => Vector {3}});
  var_dump(ImmMap {'a' => Vector {1}, 'b' => Vector {2}} ==
           ImmMap {'a' => Vector {1}, 'c' => Vector {2}});

  $fm1 = ImmMap {'x' => ImmMap {1 => 'a', 2 => 'b'},
                    'y' => ImmMap {3 => 'c', 4 => 'd'}};
  $fm2 = ImmMap {'x' => ImmMap {1 => 'a', 2 => 'b'},
                    'y' => ImmMap {3 => 'c', 4 => 'd'}};
  var_dump($fm1 == $fm2);

  $fm1 = ImmMap {'a' => Vector {1}};
  $fm2 = ImmMap {0 => $fm1};
  $fm3 = ImmMap {0 => $fm2};

  $fm4 = ImmMap {0 => ImmMap {0 => ImmMap{'a' => Vector {1}}}};
  var_dump($fm3 == $fm4);
}

main();
