<?hh

// Test equality of FixedVectors.

function main() {
  var_dump(FixedVector {} == FixedVector {});
  var_dump(FixedVector {1, 2, 3} == FixedVector {1, 2, 3});

  echo "...\n";

  var_dump(FixedVector {1, 2} == FixedVector {});
  var_dump(FixedVector {1, 2} == FixedVector {2, 1});

  echo "...\n";

  var_dump(FixedVector {1, 2} == Vector {1, 2});
  var_dump(FixedVector {Vector {1}, Vector {2}} ==
           FixedVector {Vector {1}, Vector {3}});

  echo "...\n";

  var_dump(FixedVector {FixedVector {1, 2}, Vector {3, 4}} ==
           FixedVector {FixedVector {1, 2}, Vector {3, 4}});

  $fv1 = FixedVector { Vector {1} };
  $fv2 = FixedVector{$fv1};
  $fv3 = FixedVector{$fv2};
  var_dump($fv3 == FixedVector { FixedVector { FixedVector { Vector {1}}}});
}

main();
