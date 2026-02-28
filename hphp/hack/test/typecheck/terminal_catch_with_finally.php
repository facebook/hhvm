<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function might_throw(): void {}

function test(): void {
  $x = 1;
  try {
    might_throw();
  } catch (Exception $_) {
    $x = true;
    might_throw();
    $x = 's';
    return;
  } finally {
    // Even though the catch block is terminal, $x : (int | bool | string)
    hh_show($x);
  }
  // Because the catch block is terminal, $x : int
  hh_show($x);
}
