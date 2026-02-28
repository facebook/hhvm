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

interface I2 {
  abstract const type T1 as UpperBound2;
}

// Error
trait Tr1 implements I1, I2 {
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}

// Error
trait Tr2 implements I2, I1 {
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}

// Ok
trait Tr3 implements I2, I1 {
  abstract const type T1 as UpperBound1And2;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}
