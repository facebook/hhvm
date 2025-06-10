<?hh

case type NonFixedPoint<T1> =
  | int where int as NonFixedPointOrBool<vec<T1>>;

case type NonFixedPointOrBool<T1> = NonFixedPoint<T1> | bool;
