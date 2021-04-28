<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Base { }
class Derived extends Base { }

abstract class C {
  abstract const type TC as Base;
  public function __construct(public this::TC $item) { }
}

abstract class A extends C {
  abstract const type TC as Derived;
}
abstract class D extends C {
  public function testit():Derived {
    if ($this is A) {
      $x = $this->item;
      return $x;
    }
    return new Derived();
  }
  public function testit1(this $a):Derived {
    if ($a is A) {
      $x = $a->item;
      return $x;
    }
    return new Derived();
  }
  public function testit2<Tthis as D>(Tthis $a):Derived {
    if ($a is A) {
      $x = $a->item;
      return $x;
    }
    return new Derived();
  }
}
