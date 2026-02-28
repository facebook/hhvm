<?hh

interface I {}

class ImplementsI implements I {}

abstract class Base {
  abstract const type TBase as I;
}

class Concrete extends Base {
  const type TBase = ImplementsI;
}

function the_problem<TIn as Base, TOut as I>(TIn $in): TOut
where
  TIn::TBase = TOut {
  throw new Exception();
}

function ref_it(Concrete $conrete): ImplementsI {
  $fptr = the_problem<>;
  return $fptr($conrete);
}
