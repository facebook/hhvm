<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
abstract class ACC {}
class CC extends ACC {}

function f<<<__Newable>> T as ACC>(): void {}

function g(): void {
  f<CC>();
}
