<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(arraykey $x): void {
  $v = Vector {$x};
  $v[] = 'foo';
  hh_show($v); // should be Vector<arraykey> not Vector<(arraykey | string)>
}
