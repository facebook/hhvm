<?hh

case type NonFixedPointA<T1> =
  | int where int as NonFixedPointB<vec<T1>>;

case type NonFixedPointB<T1> =
  | int where int as NonFixedPointA<vec<T1>>;
