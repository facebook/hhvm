<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function makefun1(): (function(int):bool) {
  return HH\FIXME\UNSAFE_CAST<(function(int):bool),(function(int):bool)>($x ==> true);
}

function makefun2(): (function(int):bool) {
  return $x ==> true;
}

<<__EntryPoint>>
function main():void {
  $f1 = makefun1();
  var_dump($f1(3));
  $f2 = makefun2();
  var_dump($f2(3));
}
