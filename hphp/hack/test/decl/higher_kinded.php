<?hh

type ID<T> = T;

type Test1<TC<TA1, TA2<TA3>>> = TC<int, ID>;

function filter<TC<_>, TV>(TC<TV> $collection): vec<TV> {
  return vec[];
}
