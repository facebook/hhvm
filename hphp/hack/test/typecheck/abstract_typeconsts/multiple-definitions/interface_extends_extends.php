<?hh

interface UpperBound1 {}

interface UpperBound2 extends UpperBound1 {
  abstract const type T2;
}

interface I1 {
  abstract const type T1 as UpperBound1;
}

interface I2 {
  abstract const type T1 as UpperBound2;
}

// Error
interface I3 extends I1, I2 {
  abstract const type T2 = this::T1::T2;
}

// Ok
interface I4 extends I2, I1 {
  abstract const type T2 = this::T1::T2;
}

// Ok
interface I5 extends I1, I2 {
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
