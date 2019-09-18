<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface HasFoo {
  abstract const type TFoo;
  public function getFoo(): this::TFoo;
}

abstract class C<T as HasFoo> {
  abstract const T::TFoo FOO;
}

function test<T as HasFoo>(T $x): T::TFoo {
  return $x->getFoo();
}
