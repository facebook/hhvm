<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cov<+Tc> { }
class A { }
class Base<Tb> {
  public function foo<Tf as A>():void where Tb as Cov<Tf> { }
  }
class Derived<Td> extends Base<Cov<Td>> {
  // The constraint in the superclass becomes
  //   where Cov<Td> as Cov<Tf> under instantiation
  // We must show that under this assumption,
  //   Td <: A
  // Which is true, given the constraint on Tf.
  public function foo<Tf as A>():void where Td as A { }
}
