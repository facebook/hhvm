<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Unsuccessful validation of Sealed attributes

<<__Sealed(ChildBase::class)>>
enum Base : int {
  VAL = 3;
}

// Hack and HHHVM error
enum Foo : int {
  use Base;
}

<<__EntryPoint>>
function main() : void {
  echo Foo::VAL;
}
