<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function my_example(darray<int, int> $items): void {
  $x = null as dynamic;
  foreach ($items as $x::foo) {
  }
}

<<__EntryPoint>>
function main():void {
  my_example(dict[2 => 3, 4 => 5]);
}
