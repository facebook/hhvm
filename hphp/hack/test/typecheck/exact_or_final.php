<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class D {
  public function addBranch(
    this $branch,
  ): this {
    //UNSAFE
  }
}

function testit(D $subtree):void {
  $tree = new D();
  $tree->addBranch($subtree);
}
