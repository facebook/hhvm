<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module('here'), __Internal>>
class C {
  public function bar():void { }
}
<<__Module('here')>>
interface I { }

<<__Module('here')>>
class D
// Bad
extends C
// Good
implements I {
  // Bad
  public ?C $bad1;
  // Good
  <<__Internal>> public ?C $good1;
  // Bad
  public function bad2(C $x):void { }
  // Good
  <<__Internal>> public function good2(C $c):void { }
  // Bad
  public function bad3():C { return new C(); }
  // Goood
  <<__Internal>> public function good3():C { return new C(); }
}

<<__Module('here'), __Internal>>
class E extends C {} // Ok!

<<__Module('there')>>
class F extends C {} // Bad!

class G extends C {} // Bad!
