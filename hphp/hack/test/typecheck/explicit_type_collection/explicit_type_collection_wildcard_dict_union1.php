<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
function expect(dict<arraykey,bool> $m):void { }
function testit():void {
  $m = dict<_, bool>[ 'x' => true, 1 => true ];
  hh_show($m);
  expect($m);
}
