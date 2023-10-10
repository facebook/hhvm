<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A1 implements HH\MethodAttribute {}

class TestClass {
  <<A1>>
  function decorated(): void {}
}
