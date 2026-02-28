<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(dict<string,bool> $m):void { }
function testit():void {
  $m = dict<string, _>[ 'a' => true ];
  hh_show($m);
  expect($m);
}
