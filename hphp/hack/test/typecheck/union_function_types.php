<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function testit(bool $b): void {
  if ($b) {
    $f = (int $x): string ==> "a";
  } else {
    $f = (arraykey $x): string ==> "c";
  }
  hh_show($f);
}
