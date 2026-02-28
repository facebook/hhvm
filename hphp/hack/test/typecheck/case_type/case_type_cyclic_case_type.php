<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type Recursive = Recursive | int;

case type CT = Recursive | string;

function f(CT $ct, Recursive $rec): void {
    hh_expect<CT>(''); // OK
    hh_expect<CT>($ct); // OK
    hh_expect<CT>($rec); // OK
    hh_expect<CT>(1); // Not OK?
    hh_expect<CT>(true); // Error
}
