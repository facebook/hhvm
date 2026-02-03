<?hh
<<file:__EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type A = B | int;

case type B = A | string;

function f(A $a, B $b): void {
    hh_expect<A>('');
    hh_expect<A>($a);
    hh_expect<A>($b);
    hh_expect<A>(1);
    hh_expect<A>(true);
}
