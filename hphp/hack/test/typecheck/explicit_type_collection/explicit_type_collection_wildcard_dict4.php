<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(dict<string,bool> $m):void { }
function testit():void {
  $m = dict<string, _>[ 1 => true ];
  hh_show($m);
  expect($m);
}
