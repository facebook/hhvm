<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Successful validation of Sealed attributes

<<__Sealed(ChildBase::class)>>
enum class Base : mixed {
  int VAL = 3;
}

enum class ChildBase : mixed extends Base {}

 <<__EntryPoint>>
function main() : void {
  echo ChildBase::VAL;
}
