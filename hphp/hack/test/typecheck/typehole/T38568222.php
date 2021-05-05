<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function baz(): int {
  $x = () ==> { yield 'foo'; };
  return $x();
}

<<__EntryPoint>>
function main():void {
  baz();
}
