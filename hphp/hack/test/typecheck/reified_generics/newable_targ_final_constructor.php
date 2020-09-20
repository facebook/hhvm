<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class AFC {
  final public function __construct() {}
}
class FC extends AFC {}

function f<<<__Newable>> reify T as AFC>(): void {
  new T();
}

function g(): void {
  f<FC>();
}
