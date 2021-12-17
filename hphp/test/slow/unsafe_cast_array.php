<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function dump(mixed $m):void {
  $m = HH\FIXME\UNSAFE_CAST<mixed, dict<int, int>>($m);
  var_dump($m[0]);
}

function withcast(mixed $m): void {
  HH\FIXME\UNSAFE_CAST<mixed, dict<int, int>>($m)[0] = 42;
  dump($m);
}

function withoutcast(mixed $m): void {
  $m = HH\FIXME\UNSAFE_CAST<mixed, dict<int, int>>($m);
  $m[0] = 42;
  dump($m);
}

<<__EntryPoint>>
function main():void {
  $d = dict[0 => 5];
  withcast($d);
  withoutcast($d);
}
