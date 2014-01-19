<?

function main() {
  // Precision loss when comparing bit ints w/ floats.
  var_dump((1 << 60) + 1 == (double) (1 << 60));
  var_dump((1 << 60) == (double) (1 << 60));

  var_dump(-0.0 == 0.0);
  var_dump(-0.0 == 0);
  var_dump(0.0001 == 0);
  var_dump(-0.0001 == 0);

  var_dump(-0.0 != 0.0);
  var_dump(-0.0 != 0);
  var_dump(0.0001 != 0);
  var_dump(-0.0001 != 0);

  $nan = acos(1.01);
  var_dump($nan == $nan);
  var_dump($nan != $nan);
  var_dump($nan == 0.0);
  var_dump($nan != 0.0);
}
main();
