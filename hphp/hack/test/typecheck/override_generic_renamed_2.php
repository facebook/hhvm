<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C<+T> { }

abstract class A extends B {
  public function run<Tvv as nonnull>(C<Tvv> $step):Tvv {
    throw new Exception("A");
  }
}

abstract class B
{
  public function run<Tv1 as nonnull>(
    C<Tv1> $step,
  ): Tv1 {
    throw new Exception("A");
  }
}
