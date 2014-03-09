<?hh

// Test equality of ImmVectors.

function main() {
  var_dump(ImmVector {} == ImmVector {});
  var_dump(ImmVector {1, 2, 3} == ImmVector {1, 2, 3});

  echo "...\n";

  var_dump(ImmVector {1, 2} == ImmVector {});
  var_dump(ImmVector {1, 2} == ImmVector {2, 1});

  echo "...\n";

  var_dump(ImmVector {1, 2} == Vector {1, 2});
  var_dump(ImmVector {Vector {1}, Vector {2}} ==
           ImmVector {Vector {1}, Vector {3}});

  echo "...\n";

  var_dump(ImmVector {ImmVector {1, 2}, Vector {3, 4}} ==
           ImmVector {ImmVector {1, 2}, Vector {3, 4}});

  $fv1 = ImmVector { Vector {1} };
  $fv2 = ImmVector{$fv1};
  $fv3 = ImmVector{$fv2};
  var_dump($fv3 == ImmVector { ImmVector { ImmVector { Vector {1}}}});
}

main();
