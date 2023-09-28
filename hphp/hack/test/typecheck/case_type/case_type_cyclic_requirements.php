<?hh
<<file:__EnableUnstableFeatures('case_types')>>

<<__Sealed(Trait1::class)>>
interface I1 {}

trait Trait1 {
    require implements I1;
}

case type CT = I1 | string | Vector<int>;

function f(CT $ct, I1 $i1): void {
    hh_expect<CT>(''); // OK
    hh_expect<CT>(Vector {}); // OK
    hh_expect<CT>($i1); // OK
}
