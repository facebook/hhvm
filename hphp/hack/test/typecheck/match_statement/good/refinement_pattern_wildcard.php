<?hh

<<file:__EnableUnstableFeatures('case_types')>>
<<file:__EnableUnstableFeatures('match_statements')>>

case type MyArrayKey = int | string;

function f(MyArrayKey $v): void {
  match ($v) {
    _: int => hh_expect<int>($v);
    _ => hh_expect<string>($v);
  }
}
