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
trait Tr1 {
  require implements I1;
  require implements I2;
  abstract const type T2 = this::T1::T2;
}

// Error - this will lead to a `Tany`
trait Tr2 {
  require implements I2;
  require implements I1;
  abstract const type T2 = this::T1::T2;
}

// Ok
trait Tr5 {
  require implements I2;
  require implements I1;
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
