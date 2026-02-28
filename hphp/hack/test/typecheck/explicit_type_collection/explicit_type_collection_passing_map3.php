<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(Map<string,bool> $m):void { }
function testit():void {
  $o = Map<arraykey,bool>{ 'a' => true, 42 => true };
  hh_show($o);
  expect($o);
}
