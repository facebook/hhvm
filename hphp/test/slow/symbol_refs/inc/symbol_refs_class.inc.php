<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class SomeClass {
  function __construct()[] {}
  public function foo() :mixed{ return __hhvm_intrinsics\launder_value(true); }
}
