<?hh
<<file:__EnableUnstableFeatures('case_types')>>

<<__Sealed(I2::class)>>
interface I1 {}

<<__Sealed(I1::class)>>
interface I2 extends I1 {}

case type CT = I1 | string | Vector<int>;

function f(CT $ct, I1 $i1, I2 $i2): void {
    hh_expect<CT>(''); // OK
    hh_expect<CT>(Vector {}); // OK
    hh_expect<CT>($i1); // OK
    hh_expect<CT>($i2); // OK
}
