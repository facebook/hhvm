<?hh

type A = shape('a' => int, 'b' => int);
type B = shape('a' => A, 'b' => A);
type C = shape('a' => B, 'b' => B);
type D = shape('a' => C, 'b' => C);
type E = shape('a' => D, 'b' => D);
type F = shape('a' => E, 'b' => E);
type G = shape('a' => F, 'b' => F);
type H = shape('a' => G, 'b' => G);
type I = shape('a' => H, 'b' => H);
type J = shape('a' => I, 'b' => I);
type K = shape('a' => J, 'b' => J);
type L = shape('a' => K, 'b' => K);
type M = shape('a' => L, 'b' => L);
type N = shape('a' => M, 'b' => M);

function f(N $a): void {
  g($a);
}

function g(int $_): void {}
