<?hh

interface UpperBound1 {
  abstract const type T2;
}

interface UpperBound2 {
  abstract const type T3;
}

interface UpperBound1And2 extends UpperBound1, UpperBound2 {}

abstract class C1 {
  abstract const type T1 as UpperBound1;
}

abstract class C2 {
  abstract const type T1 as UpperBound2;
}

// Error
interface I1 {
  require extends C1;
  require extends C2;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}

// Error
interface I2 {
  require extends C2;
  require extends C1;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}

// Ok
interface I3 {
  require extends C2;
  require extends C1;
  abstract const type T1 as UpperBound1And2;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}
