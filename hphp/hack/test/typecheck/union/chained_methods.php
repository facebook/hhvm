<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function validateFilterParams(
  bool $b,
  ?Vector<int> $vi
  ): void {

  $si = $vi !== null ? $vi->toSet()
      : Set {};
  if (
    $b &&
    !$si->remove(0)->isEmpty()
    ) {
      throw new Exception("A");
    }
}
