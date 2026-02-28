<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(dict<string,bool> $m):void { }
function testit():void {
  $m = dict<_, bool>[ 'a' => 4 ];
  hh_show($m);
  expect($m);
}
