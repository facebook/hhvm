<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}
namespace Test {

  function getMissing(): ~\HH_FIXME\MISSING_TYPE_IN_HIERARCHY {
    return "A";
  }

  function expectLikeInt(~int $x): void {}
  function testforeach(): void {
    $x = getMissing();
    if ($x is nonnull) {
      foreach ($x as $y) {
        expectLikeInt($y);
      }
    }
  }
}
