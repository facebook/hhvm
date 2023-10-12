<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function expectString(string $x): void {
  echo 'expectString';
}

function test(bool $b): int {
  $x = 'a';
  try {
    if ($b) {
      $x = true;
      return 0;
    }
  } finally {
    expectString($x);
  }
  return 2;
}
