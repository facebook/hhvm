<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// ok
function f1(): void {
  $f = async (): AsyncGenerator<null, int, void> ==> {
    yield 5;
  };
}

// error
function f2(): void {
  $f = (): AsyncGenerator<null, int, void> ==> {
    yield 5;
  };
}

// error
function f3(): void {
  $f = async (): Generator<null, int, void> ==> {
    yield 5;
  };
}
