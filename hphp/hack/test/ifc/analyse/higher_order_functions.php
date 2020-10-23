<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__InferFlows>>
function apply((function(int): int) $f): int {
  return $f(0);
}
