<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(darray<string,bool> $m):void { }
function testit():void {
  $n = dict<arraykey,bool>[];
  hh_show($n);
  // Likewise this
  expect($n);
}
