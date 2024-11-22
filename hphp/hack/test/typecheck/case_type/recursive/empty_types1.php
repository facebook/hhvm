<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type A = A;
case type B = ?B;

case type C = D;
case type D = C;

case type E = F;
case type F = G;
case type G = E;

case type H = I;
case type I = ?H;

case type J = K;
case type K = ?L;
case type L = J;

case type M = M | int;
case type N = ~N;

case type Mix = mixed;
case type NN = nonnull;
case type Ref = Ref with { type T = Ref };
case type Access = ?Access::T;
case type Accesss = Accesss::T;
case type Acces = Claz::T | int;
case type Accez = ?Clazz::T;

class Claz {
  const type T = Acces;
}
class Clazz {
  const type T = Accez;
}
