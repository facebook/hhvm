<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class AFC {
  final public function __construct() {}
}
class FC extends AFC {}

function f<<<__Newable>> T as AFC>(): void {}

function g(): void {
  f<FC>();
}
