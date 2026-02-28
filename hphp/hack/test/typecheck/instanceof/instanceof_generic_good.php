<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<Tinv> {}
class Cov<+Tcov> {}
class Contra<-Tcontra> {}

interface I {}
class Base implements I {}
class Derived extends Base {}
class C1 extends Inv<Base> {}
class C2 extends Cov<Base> {}
class C3 extends Contra<Base> {}

function ExpectsBase(Base $b): void {}
function ExpectsDerived(Derived $d): void {}
function ExpectsI(I $i): void {}
function Test<T>(Inv<T> $cinv, Cov<T> $ccov, Contra<T> $ccontra, T $t): ?T {
  if ($cinv is C1) {
    // Here we expect $cinv to have type C1
    // Hence C1 <: Inv<T> so Inv<Base> <: Inv<T>
    // Hence T = Base
    ExpectsBase($t);
  }
  if ($ccov is C2) {
    // Here we expect $ccov to have type C2
    // Hence C2 <: Cov<T>
    // But C2 <:: Cov<Base> uniquely
    // So Cov<Base> <: Cov<T>
    // Hence Base <: T
    return $t;
  }
  if ($ccontra is C3) {
    // Here we expect $ccontra to have type C3
    // Hence C3 <: Contra<T>
    // But C3 <:: Contra<Base> uniquely
    // So Contra<Base> <: Contra<T>
    // Hence T <: Base
    ExpectsBase($t);
  }
  return null;
}
