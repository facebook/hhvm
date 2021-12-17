<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function non_empty(string $s): ?string {
  return $s ? $s : null;
}
