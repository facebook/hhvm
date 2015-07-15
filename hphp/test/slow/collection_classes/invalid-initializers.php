<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test() {
  try {
    echo "Creating Set with booleans...\n";
    $foo = Set { true, false };
    echo "Set with booleans succeeded!\n";
  } catch (InvalidArgumentException $exn) {
    echo "Set with booleans failed: \"" . $exn->getMessage() . "\"\n";
  }

  try {
    echo "Creating Set with floats...\n";
    $foo = Set { 1.234, 5.6789 };
    echo "Set with floats succeeded!\n";
  } catch (InvalidArgumentException $exn) {
    echo "Set with floats failed: \"" . $exn->getMessage() . "\"\n";
  }

  try {
    echo "Creating Map with booleans...\n";
    $foo = Map { true => 'a', false => 'b' };
    echo "Map with booleans succeeded!\n";
  } catch (InvalidArgumentException $exn) {
    echo "Map with booleans failed: \"" . $exn->getMessage() . "\"\n";
  }

  try {
    echo "Creating Map with floats...\n";
    $foo = Map { 1.234 => 'a', 5.6789 => 'b' };
    echo "Map with floats succeeded!\n";
  } catch (InvalidArgumentException $exn) {
    echo "Map with floats failed: \"" . $exn->getMessage() . "\"\n";
  }
}

test();
