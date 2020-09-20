<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function apply((function(int): int) $f): int {
  return $f(0);
}
