<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function might_throw(): void {}

function test(): void {
  $x = 1;
  try {
    might_throw();
    $x = true;
    return;
  } catch (Exception $_) {
    $x = 's';
  } finally {
    // Even though the try block is terminal, $x : (int | bool | string)
    hh_show($x);
  }
  // Because the try block is terminal, $x : string
  hh_show($x);
}
