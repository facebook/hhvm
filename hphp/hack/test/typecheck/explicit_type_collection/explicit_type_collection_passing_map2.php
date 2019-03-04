<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
function expectMapStringBool(Map<string,bool> $m):void { }
function testit():void {
  $n = Map<arraykey,bool>{};
  hh_show($n);
  // Likewise this
  expectMapStringBool($n);
}
