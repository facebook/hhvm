<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(): void {
  $f = $x ==> {
    if (false) {
      return $x;
    } else {
      return;
    }
  };
  $f(42);
}
