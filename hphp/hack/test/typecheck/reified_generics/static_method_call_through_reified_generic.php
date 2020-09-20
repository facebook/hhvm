<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test<reify T as arraykey>(): void {
  T::foo();
}
