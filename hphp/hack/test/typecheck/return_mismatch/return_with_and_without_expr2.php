<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(bool $b): void {
  $f = $x ==> {
    if ($b) {
      return $x;
    } else {
      return;
    }
  };

  return $f(42);
}
