<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function test(arraykey $x): void {
  match ($x) {
    _ => hh_expect<arraykey>($x);
    _ => hh_expect<nothing>($x);
  }
}
