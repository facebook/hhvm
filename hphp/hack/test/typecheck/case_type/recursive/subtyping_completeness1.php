<?hh

case type A = vec<A>;
case type B = vec<B>;
case type C = vec<vec<C>>;

function test(A $a, B $b, C $c): void {
  hh_expect<A>($b);
  hh_expect<A>($c);
  hh_expect<C>($a);
}
