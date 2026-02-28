<?hh

<<file:__EnableUnstableFeatures('newtype_super_bounds')>>

newtype N<+T> as T = T; // ok
newtype B as N<B> = N<nothing>;

class C {
  const type T = D;
}
newtype D as C::T = int;

class H {
  const type T = I;
}
newtype I as I::T = H;

newtype E as ?E = null;
newtype F as vec<F> = vec<nothing>;
newtype G as shape(?'g' => G) = shape();
newtype J as this = nothing;
newtype M super M = int;
