<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type CT<T1 as T2, T2 as T1 as arraykey> = vec<T2> | T1;

function f1(CT<int, int> $c): void {
    if ($c is vec<_>) {
        hh_expect_equivalent<vec<int>>($c);
    } else {
        hh_expect_equivalent<int>($c);
    }
}

function f2(CT<int, int> $c): void {
    if ($c is int) {
        hh_expect_equivalent<int>($c);
    } else {
        hh_expect_equivalent<vec<int>>($c);
    }
}
