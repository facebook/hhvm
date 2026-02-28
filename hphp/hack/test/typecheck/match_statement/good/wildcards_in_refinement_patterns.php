<?hh

<<file:__EnableUnstableFeatures('case_types')>>
<<file:__EnableUnstableFeatures('match_statements')>>

case type C = int | vec<string> | dict<string, int>;

function test(C $x): void {
  match ($x) {
    _: int => hh_expect<int>($x);
    _: vec<_> => hh_expect<vec<string>>($x);
    _: dict<_, _> => hh_expect<dict<string, int>>($x);
  }
}
