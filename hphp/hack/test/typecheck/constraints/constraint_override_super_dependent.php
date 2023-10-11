<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Base<Tv> {

  abstract public function foo<Tu super Tv>(Tv $item): ?Tu;
}

class C {}
abstract class Derived extends Base<this::TC> {
  abstract const type TC as C;
  // We should be checking that assuming Tu super this::TC
  // we have Tu super this::TC !!
  // But this::TC is getting resolved...
  public function concatenate<Tu super this::TC>(this::TC $item): ?this::TC {
    return $item;
  }
}
