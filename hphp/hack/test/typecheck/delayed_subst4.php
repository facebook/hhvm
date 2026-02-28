<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1<T> {
  public function call(T $in): bool;
}

interface I2<T> extends I1<T> {}

abstract class TestXXX<T> implements I1<T>, I2<T> {}
