<?hh
// Copyright 2020-present Facebook. All Rights Reserved.

function badInt(bool $b): int {
  if ($b) {
    $x = 3;
  } else {
    $x = 3.4;
  }
  $z = 3 * $x;
  if ($z is int) {
    echo 'int';
  } else {
    echo 'not int';
  }
  return $z;
}

function main(): void {
  badInt(false);
}
