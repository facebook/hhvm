<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__EntryPoint>>
function main(): void {
  echo "Testing HH\\Sandbox\\dict_with_capacity...\n";

  // Test basic functionality
  $d = HH\Sandbox\dict_with_capacity(1000);
  echo "Created empty dict with capacity 1000\n";
  echo "Dict count: ".count($d)."\n";

  // Populate the dict
  for ($i = 0; $i < 100; $i++) {
    $d[$i] = $i * 2;
  }
  echo "After adding 100 elements, count: ".count($d)."\n";

  // Verify values
  if ($d[50] === 100) {
    echo "Dict value verification passed\n";
  } else {
    echo "Dict value verification FAILED\n";
  }

  echo "\nTesting HH\\Sandbox\\vec_with_capacity...\n";

  // Test Vec
  $v = HH\Sandbox\vec_with_capacity(500);
  echo "Created empty vec with capacity 500\n";
  echo "Vec count: ".count($v)."\n";

  // Populate the vec
  for ($i = 0; $i < 100; $i++) {
    $v[] = $i;
  }
  echo "After adding 100 elements, count: ".count($v)."\n";

  // Verify values
  if ($v[50] === 50) {
    echo "Vec value verification passed\n";
  } else {
    echo "Vec value verification FAILED\n";
  }

  echo "\nTesting error handling...\n";
  try {
    $bad = HH\Sandbox\dict_with_capacity(0);
    echo "ERROR: Should have thrown for capacity 0\n";
  } catch (InvalidArgumentException $e) {
    echo "Correctly threw InvalidArgumentException for capacity 0\n";
  }

  try {
    $bad = HH\Sandbox\dict_with_capacity(-1);
    echo "ERROR: Should have thrown for capacity -1\n";
  } catch (InvalidArgumentException $e) {
    echo "Correctly threw InvalidArgumentException for capacity -1\n";
  }

  echo "\nAll tests passed!\n";
}
