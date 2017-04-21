<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function convert_alot($a) {
  for ($i = 0; $i < 10000; $i++) {
    try { vec($a); } catch (Exception $e) {}
  }
}

function main() {
  $value = 0;
  $arr = [];

  // Packed array
  for ($i = 0; $i < 5000; $i++) {
    $arr[] = &$value;
  }
  convert_alot($arr);

  // Mixed array
  unset($arr[500]);
  convert_alot($arr);
}

ini_set('memory_limit', '100K');
main();
echo "OK\n";
