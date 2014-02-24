<?hh

// Test equality of FrozenVectors.

function main() {
  var_dump(FrozenVector {} == FrozenVector {});
  var_dump(FrozenVector {1, 2, 3} == FrozenVector {1, 2, 3});

  echo "...\n";

  var_dump(FrozenVector {1, 2} == FrozenVector {});
  var_dump(FrozenVector {1, 2} == FrozenVector {2, 1});

  echo "...\n";

  var_dump(FrozenVector {1, 2} == Vector {1, 2});
  var_dump(FrozenVector {Vector {1}, Vector {2}} ==
           FrozenVector {Vector {1}, Vector {3}});

  echo "...\n";

  var_dump(FrozenVector {FrozenVector {1, 2}, Vector {3, 4}} ==
           FrozenVector {FrozenVector {1, 2}, Vector {3, 4}});

  $fv1 = FrozenVector { Vector {1} };
  $fv2 = FrozenVector{$fv1};
  $fv3 = FrozenVector{$fv2};
  var_dump($fv3 == FrozenVector { FrozenVector { FrozenVector { Vector {1}}}});
}

main();
