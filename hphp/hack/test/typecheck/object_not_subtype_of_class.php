<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {}

function test<T>(classname<T> $c): C {
  return new $c();
}
