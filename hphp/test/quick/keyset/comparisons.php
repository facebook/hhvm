<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function single_compare($a, $b) {
  echo "Comparing:\n";
  var_dump($a);
  var_dump($b);

  echo "\t<   : ";
  try {
    var_dump($a < $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t<=  : ";
  try {
    var_dump($a <= $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t>   : ";
  try {
    var_dump($a > $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t>=  : ";
  try {
    var_dump($a >= $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t==  : ";
  try {
    var_dump($a == $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t!=  : ";
  try {
    var_dump($a != $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t=== : ";
  try {
    var_dump($a === $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t!== : ";
  try {
    var_dump($a !== $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t<=> : ";
  try {
    var_dump($a <=> $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }
}

function compare($a, $b) {
  single_compare($a, $b);
  single_compare($b, $a);
}

function main() {
  single_compare(keyset[], keyset[]);
  single_compare(keyset[1, 2, 3], keyset[1, 2, 3]);
  compare(keyset[4, 5], keyset[4, 6]);
  compare(keyset[], keyset[1, 2]);
  compare(keyset[1, 2, 3], keyset[3, 2, 1]);
  compare(keyset['a', 'b', 'c'], keyset['c', 'b', 'a']);

  $ks = keyset['a', 'b'];
  single_compare($ks, $ks);

  compare(keyset[12345], keyset["12345"]);

  compare(keyset[], null);
  compare(keyset[], false);
  compare(keyset[], 123);
  compare(keyset[], 1.2345);
  compare(keyset[], 'abc');
  compare(keyset[], new stdclass);
  compare(keyset[], []);
  compare(keyset[], vec[]);
  compare(keyset[], dict[]);
}
main();
