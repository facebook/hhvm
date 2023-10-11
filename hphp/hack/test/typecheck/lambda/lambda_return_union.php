<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expectArrayKey(arraykey $ak): void {}
function f(): void {
  $f = (int $x, bool $b) ==> {
    if ($b)
      return $x;
    else
      return 'a';
  };
  expectArrayKey($f(2, true));
}
