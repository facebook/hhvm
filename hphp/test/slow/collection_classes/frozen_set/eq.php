<?hh

// Test that ImmSets can be compared for equality.

function main() {
  $fs1 = new ImmSet(Vector {1, 2, 3});
  $fs2 = new ImmSet(Vector {3, 2, 1});
  var_dump($fs1 == $fs2);

  $s = new Set(Vector{2, 3, 1});
  var_dump($s == $fs1);
  var_dump($fs2 == $s);
}

main();
