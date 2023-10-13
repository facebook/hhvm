<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(dict<arraykey,int> $d):(function(arraykey):int) {
  $f = $x ==> $d[$x];
  return $f;
}
