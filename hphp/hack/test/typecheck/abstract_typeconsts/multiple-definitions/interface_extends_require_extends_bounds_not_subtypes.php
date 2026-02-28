<?hh

interface UpperBound1 {
  abstract const type T2;
}

interface UpperBound2 {
  abstract const type T3;
}

interface UpperBound1And2 extends UpperBound1, UpperBound2 {}

interface I1 {
  abstract const type T1 as UpperBound1;
}

abstract class C1 {
  abstract const type T1 as UpperBound2;
}

interface I2 {
  abstract const type T1 as UpperBound2;
}

abstract class C2 {
  abstract const type T1 as UpperBound1;
}

// Error
interface I3 extends I1 {
  require extends C1;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}

// Error
interface I4 extends I2 {
  require extends C2;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}

// Ok
interface I5 extends I2 {
  require extends C2;
  abstract const type T1 as UpperBound1And2;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}
