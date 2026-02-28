<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function g<Ta>((function(int):Ta) $f): Ta {
  return $f(3);
}

function testit():void {
  $r = g($x ==> $x+1);
}
