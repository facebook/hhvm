<?hh

case type NonFixedPoint<T1> =
  | int where int as NonFixedPoint<vec<T1>>;

function foo(): NonFixedPoint<int> {
  return 1;
}
