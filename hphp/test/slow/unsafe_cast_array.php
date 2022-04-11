<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.

function dump(mixed $m):void {
  $m = HH\FIXME\UNSAFE_CAST<mixed, dict<int, int>>($m);
  var_dump($m[0]);
}

<<__EntryPoint>>
function main():void {
  $d = dict[0 => 5];
  HH\FIXME\UNSAFE_CAST<mixed, dict<int, int>>($m)[0] = 42;
  dump($d);
}
