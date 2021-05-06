<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait FooTrait {}

function hgoldstein<<<__Enforceable>> reify T>(mixed $in): void {
  $in as T;
  hgoldstein<FooTrait>($in);
}

<<__EntryPoint>>
function main():void {
  hgoldstein<FooTrait>(null);
}
