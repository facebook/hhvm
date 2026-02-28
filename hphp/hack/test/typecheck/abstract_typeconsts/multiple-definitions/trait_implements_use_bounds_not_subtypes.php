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

trait Tr1 {
  abstract const type T1 as UpperBound2;
}

interface I2 {
  abstract const type T1 as UpperBound2;
}

trait Tr2 {
  abstract const type T1 as UpperBound1;
}

// Error
trait Tr3 implements I1 {
  use Tr1;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}

// Error
trait Tr4 implements I2 {
  use Tr2;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}

// Ok
trait Tr5 implements I1 {
  use Tr1;
  abstract const type T1 as UpperBound1And2;
  abstract const type T2 = this::T1::T2;
  abstract const type T3 = this::T1::T3;
}
