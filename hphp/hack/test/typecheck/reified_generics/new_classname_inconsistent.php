<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

function f<T as A>(classname<T> $a): void {
  new $a();
}
