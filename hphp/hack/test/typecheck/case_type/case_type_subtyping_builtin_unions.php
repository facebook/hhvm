<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type CT = int | string | float | null;

function ct(CT $c): void {
}

enum E : int {
    N = 1;
}

function good<T as arraykey as num>(arraykey $ak, num $n, ?num $nn, ?arraykey $nak, T $t): void {
    ct($ak);
    ct($n);
    ct($nn);
    ct($nak);
    ct(E::N);
    ct($t);
}
