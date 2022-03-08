<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.
// @format

function test1():dict<int,string> {
  return dict[2 => HH\FIXME\UNSAFE_CAST<mixed, string>("A")];
}
function test2():dict<int,string> {
  return dict[2 => "A"];
}

<<__EntryPoint>>
function main():void {
  $x1 = test1();
  var_dump($x1);
  $x2 = test2();
  var_dump($x2);
}
