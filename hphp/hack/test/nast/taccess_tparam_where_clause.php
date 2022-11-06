<?hh

abstract class C {
  abstract const type TA;
}

function taccess_tparam_where_clause<T1, T2>(T1 $x, T2 $y): void
where
  T1 as C,
  T2 as T1::TA {}
