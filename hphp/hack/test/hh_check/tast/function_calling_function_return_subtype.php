<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function g(): int {
  return g();
}

function f(): num {
  return g();
}
