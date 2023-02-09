<?hh

type A = shape('a1' => int, 'a2' => int);
type B = shape('b1' => A, 'b2' => A);
type C = shape('c1' => B, 'c2' => B);
type D = shape('d1' => C, 'd2' => C);
type E = shape('e1' => D, 'e2' => D);
type F = shape('f1' => E, 'f2' => E);
type G = shape('g1' => F, 'g2' => F);
type H = shape('h1' => G, 'h2' => G);
type I = shape('i1' => H, 'i2' => H);
type J = shape('j1' => I, 'j2' => I);
type K = shape('k1' => J, 'k2' => J);
type L = shape('l1' => K, 'l2' => K);
type M = shape('m1' => L, 'm2' => L);
type N = shape('n1' => M, 'n2' => M);

function f(N $a): void {
  g($a);
}

function g(int $_): void {}
