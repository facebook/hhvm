<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(Map<string,bool> $m):void { }
function testit():void {
  $m = Map<arraykey,bool>{ 'a' => true };
  hh_show($m);
  expect($m);
}
