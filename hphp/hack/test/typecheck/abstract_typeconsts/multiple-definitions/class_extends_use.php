<?hh

interface UpperBound1 {}

interface UpperBound2 extends UpperBound1 {
  abstract const type T2;
}

abstract class C1 {
  abstract const type T1 as UpperBound1;
}

trait Tr1 {
  abstract const type T1 as UpperBound2;
}

abstract class C2 {
  abstract const type T1 as UpperBound2;
}

trait Tr2 {
  abstract const type T1 as UpperBound1;
}

// Ok
abstract class C3 extends C1 {
  use Tr1;
  abstract const type T2 = this::T1::T2;
}

// Error
abstract class C4 extends C2 {
  use Tr2;
  abstract const type T2 = this::T1::T2;
}

// Ok
abstract class C5 extends C2 {
  use Tr2;
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
