<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type Scalars = int | bool | string | float | null;

case type Bad = Scalars | E;

enum E : F as F {
    A = F::A;
}

enum F : int as int {
    A = 1;
}
