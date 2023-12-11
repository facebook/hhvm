<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

function array_cse() :mixed{
  $a = vec[0,1,2,3,4];
  $x = $a[2] + $a[2];
  return $x;
}
<<__EntryPoint>> function main(): void {
var_dump(array_cse());
}
