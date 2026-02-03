<?hh

interface UpperBound1 {}

interface UpperBound2 extends UpperBound1 {
  abstract const type T2;
}

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

// Error - this will lead to a `Tany`
trait Tr1 {
  require implements I1;
  require extends C1;
  abstract const type T2 = this::T1::T2;
}

// Ok
trait Tr2 {
  require implements I2;
  require extends C2;
  abstract const type T2 = this::T1::T2;
}

// Ok
trait Tr3 {
  require implements I1;
  require extends C1;
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
