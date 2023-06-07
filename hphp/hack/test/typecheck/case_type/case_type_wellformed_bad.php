<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type Bad1 = vec<int> | Traversable<string>;

case type Good1 = vec<int> | dict<int, int>;

interface I1 {}

interface I2<T> {}

final class C {}

case type Bad2 = I1 | I2<int>;

case type Good2 = C | I2<int>;

case type Good3 = C | I1;

case type Bad3 = vec<int> | (int, int);

case type Good4 = vec<int> | shape('x' => int);

case type Bad4 = dict<string, int> | shape('x' => int);
