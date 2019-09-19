<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class D {
  /* HH_FIXME[4336] */
  public function addBranch(
    this $branch,
  ): this {
  }
}

function testit(D $subtree):void {
  $tree = new D();
  $tree->addBranch($subtree);
}
