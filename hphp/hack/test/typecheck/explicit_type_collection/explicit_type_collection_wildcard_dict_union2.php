<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(dict<string, arraykey> $m):void { }
function testit():void {
  $m = dict<string, _>[ 'x' => 1, 'y' => '!' ];
  hh_show($m);
  expect($m);
}
