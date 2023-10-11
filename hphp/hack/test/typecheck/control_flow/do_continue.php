<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expectString(string $s): void {}

function test_flow(): void {
  $i = 0;
  $s = "a";
  // At this point $s is a string
  do {
    if ($i < 5) {
      // Here it might be a float!
      expectString($s);
      $i++;
      $s = 5.4;
      continue;
    }
    $i++;
  } while ($i < 10);
}
