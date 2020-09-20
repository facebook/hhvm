<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {}
class Contra<-T> {}

class TestClass {
  // Return something so we can see the type that's inferred for the parameters
  /* HH_FIXME[4110] */
  final public function testContra<T>(T $x, Contra<T> $y): T {
  }
  public function main(?I $ent, Contra<I> $p): void {
    // Must have following constraints
    //   (1) ?I <: T
    //   (2) Contra<I> <: Contra<T> so therefore T <: I
    // What possible solution do we have for T?
    //  * Consider T = I. NO, because we don't have ?I <: I for (1)
    //  * Consider T = ?I. NO, because we don't have ?I <: I for (2)
    $r = $this->testContra($ent, $p);
    //    hh_show_env();
  }
}
