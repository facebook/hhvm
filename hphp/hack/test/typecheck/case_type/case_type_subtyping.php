<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type CT<T> = int | bool | vec<T> | Vector<T>;

function ct(CT<int> $c): void {
}

function good(): void {
    ct(10);
    ct(true);
    ct(vec[10]);
    ct(Vector { 10 });
}

function bad(): void {
    ct(10.0);
    ct("true");
    ct(vec[null]);
    ct(Vector { vec[10] });
}
