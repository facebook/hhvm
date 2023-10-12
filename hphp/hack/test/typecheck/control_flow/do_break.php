<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_flow(): string {
  $i = 0;
  $s = "ho";
  do {
    if ($i < 5) {
      $s = 5;
      break;
    }
    $s = "hey";
    $i++;
  } while ($i < 10);
  // Because of the break statement $s has wrong type here
  return $s;
}
