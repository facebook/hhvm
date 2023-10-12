<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class MyList<T> {
  public function __construct(public T $item) { }
}
final class C<Tc> {
  public function thisisok1<T>(T $x):T { return $x; }
  public function thisisok2<T>():void where Tc = MyList<T> { }
  public function thisisok3<T>(inout T $_x):void { }
  public function thisisok4<T>():(function(inout T):void) {
    throw new Exception("E");
  }
  public function thisisok5<reify T>(T $_x):void { }
  public function thisisok6<reify T>():T {
    throw new Exception("E");
  }
  public function thisisok7<<<__Explicit>> T>(T $_x):void { }
  public function thisisok8<<<__Explicit>> T>():T {
    throw new Exception("E");
  }
  public function thisisok9<T>(T... $x): T {
    throw new Exception("E");
  }
}

final class Cov<+TCov> {
  public function concat1<T super TCov>(T $_x):vec<T> { return vec[]; }
  public function concat2<T>(T $_x):vec<T> where T super TCov { return vec[]; }
}

interface HasFoo {
  abstract const type TFoo;
  public function getFoo(): this::TFoo;
}

function thisisok<T as HasFoo, TFoo>(T $x): TFoo where TFoo = T::TFoo {
  return $x->getFoo();
}
