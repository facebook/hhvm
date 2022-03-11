<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectTrav<T>(~Traversable<T> $_):void { }
function pass(supportdyn<Traversable<mixed>> $x):void {
  expectTrav($x);
}
