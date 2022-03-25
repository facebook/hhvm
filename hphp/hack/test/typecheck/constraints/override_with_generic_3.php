<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Foo { }

interface IMyEnt {
  abstract const type TC as Foo;
}

class MyBase {
  public function do():Foo {
    throw new Exception("A");
  }
}

final class MyReified<reify TEnt as IMyEnt> extends MyBase {
    <<__Override>>
    public function do<T>():T where T = TEnt::TC {
      throw new Exception("A");
    }
}
