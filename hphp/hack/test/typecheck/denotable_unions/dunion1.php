<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function foo(bool $b): (int | bool) {
  return $b ? 3 : $b;
}
