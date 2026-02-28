<?hh

final class Cov<+T> {}

case type NonFixedPoint<T1> =
| string where Cov<string> as Cov<NonFixedPoint<vec<T1>>>;
