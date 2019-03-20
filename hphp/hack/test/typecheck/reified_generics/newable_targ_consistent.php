<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__ConsistentConstruct>>
abstract class ACC {}
class CC extends ACC {}

function f<<<__Newable>> reify T as ACC>(): void {
  new T();
}

function g(): void {
  f<CC>();
}
