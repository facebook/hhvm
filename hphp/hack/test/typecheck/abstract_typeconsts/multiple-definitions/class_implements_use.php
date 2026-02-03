<?hh

interface UpperBound1 {}

interface UpperBound2 extends UpperBound1 {
  abstract const type T2;
}

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
abstract class C3 implements I1 {
  use Tr1;
  abstract const type T2 = this::T1::T2;
}

// Ok
abstract class C4 implements I2 {
  use Tr2;
  abstract const type T2 = this::T1::T2;
}

// Ok
abstract class C5 implements I2 {
  use Tr2;
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
