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
trait Tr3 {
  require extends C1;
  use Tr1;
  abstract const type T2 = this::T1::T2;
}

// Error - this will lead to a `Tany`
trait Tr4 {
  require extends C2;
  use Tr2;
  abstract const type T2 = this::T1::T2;
}

// Ok
trait Tr5 {
  require extends C2;
  use Tr2;
  abstract const type T1 as UpperBound2;
  abstract const type T2 = this::T1::T2;
}
