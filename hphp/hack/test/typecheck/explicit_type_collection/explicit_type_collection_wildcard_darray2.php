<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(darray<string,bool> $m):void { }
function testit():void {
  $m = darray<_,bool>[ 'a' => 4 ];
  hh_show($m);
  expect($m);
}
