<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
function expectMapStringBool(darray<string,bool> $m):void { }
function testit():void {
  $o = darray<arraykey,bool>[ 'a' => true, 42 => true ];
  hh_show($o);
  // I expect this to fail
  expectMapStringBool($o);
}
