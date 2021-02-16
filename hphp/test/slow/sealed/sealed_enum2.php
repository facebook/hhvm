<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Successful validation of Sealed attributes

<<__Sealed(ChildBase::class)>>
enum Base : int {
  VAL = 3;
}

enum ChildBase : int {
  use Base;
}

<<__EntryPoint>>
function main() : void {
  echo ChildBase::VAL;
}
