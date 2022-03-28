<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait TR {
  protected abstract function foo<Tv as nonnull>(C<Tv> $step): Tv;
}

final class FC extends AC<void> {
  use TR;
}

abstract class AC<Tv> extends C<Tv> {
  // Hack will rename foo's method type parameters away from Tv
}

abstract class C<+Tstep> {
  public function foo<Tv as nonnull>(C<Tv> $step): Tv {
    throw new Exception("A");
  }
}
