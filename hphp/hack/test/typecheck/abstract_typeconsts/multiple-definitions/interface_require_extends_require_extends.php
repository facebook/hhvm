<?hh

interface UpperBound1 {}

interface UpperBound2 extends UpperBound1 {
  abstract const type T2;
}

abstract class C1 {
  abstract const type T1 as UpperBound1;
}

abstract class C2 {
  abstract const type T1 as UpperBound2;
}

// Ok
interface I1 {
  require extends C1;
  require extends C2;
  abstract const type T2 = this::T1::T2;
}

// Error - this::T1::T2 will result in a `Tany`
interface I2 {
  require extends C2;
  require extends C1;
  abstract const type T2 = this::T1::T2;
}

// Ok
interface I3 {
  require extends C2;
  require extends C1;
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
