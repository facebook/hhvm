<?hh

abstract class C {
  abstract const type TA;
}

function taccess_tparam<T>(T $x, T::TA $y): void
where
  T1 as C {}
