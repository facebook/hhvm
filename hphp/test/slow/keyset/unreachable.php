<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  $a = keyset['a', 'b', false, 3, 4];
  var_dump($a);
}


<<__EntryPoint>>
function main_unreachable() {
try {
  main();
} catch (Exception $e) {
  echo $e->getMessage() . "\n";
}
}
