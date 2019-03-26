<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// If we were in the middle of initializing a dict, but realized one of the
// values is a ref, we'd throw an exception, but fail to deallocate the
// allocated but not fully initialized dict.

function convert_alot($a) {
  for ($i = 0; $i < 10000; $i++) {
    try { dict($a); } catch (Exception $e) {}
  }
}

function main() {
  $value = 0;
  $arr = [&$value];

  // Packed array
  for ($i = 0; $i < 5000; $i++) {
    $arr[] = $value;
  }
  convert_alot($arr);

  // Mixed array
  unset($arr[500]);
  convert_alot($arr);
}


<<__EntryPoint>>
function main_ref_fail_leak() {
ini_set('memory_limit', '18M');
main();
echo "OK\n";
}
