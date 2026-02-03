<?hh

interface UpperBound1 {}

interface UpperBound2 extends UpperBound1 {
  abstract const type T2;
}

trait Tr1 {
  abstract const type T1 as UpperBound1;
}

trait Tr2 {
  abstract const type T1 as UpperBound2;
}

// Ok
trait Tr3 {
  use Tr1;
  use Tr2;
  abstract const type T2 = this::T1::T2;
}

// Error
trait Tr4 {
  use Tr2;
  use Tr1;
  abstract const type T2 = this::T1::T2;
}

// Ok
trait Tr5 {
  use Tr2;
  use Tr1;
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
