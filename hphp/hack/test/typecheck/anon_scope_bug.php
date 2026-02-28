<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(int $x): void {
  function () {
    $x; // should be undefined
  };
}
