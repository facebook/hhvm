<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function main(): void {
  $f = (): mixed ==> {
    yield 5;
  };
}
