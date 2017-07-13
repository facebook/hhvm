<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  $a = keyset['a', 'b', false, 3, 4];
  var_dump($a);
}

try {
  main();
} catch (Exception $e) {
  echo $e->getMessage() . "\n";
}
