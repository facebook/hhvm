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

// Ok
abstract class C1 implements I1, I2 {
  abstract const type T2 = this::T1::T2;
}

// Error
abstract class C2 implements I2, I1 {
  abstract const type T2 = this::T1::T2;
}

// Ok
abstract class C3 implements I2, I1 {
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
