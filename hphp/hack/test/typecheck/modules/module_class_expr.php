<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module('here'), __Internal>>
class C {
  public function bar(mixed $m):void {
    // All not ok
    $x1 = new D();
    $y1 = D::class;
    D::foo();
    // All ok
    $x2 = new C();
    $y2 = C::class;
    C::foo();
    $x3 = new self();
    $y3 = self::class;
    self::foo();
  }
  public static function foo():void { }
}

<<__Module('there'), __Internal>>
class D {
  public static function foo():void { }
}
