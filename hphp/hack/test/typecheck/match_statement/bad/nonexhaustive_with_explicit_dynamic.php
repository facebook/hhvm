<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function test(num $x): void {
  if (coinflip()) $x = get_dynamic(); // $x: (num | dynamic)
  match ($x) {
    _: int => hh_expect<int>($x);
  }
}

function coinflip(): bool { return false; }
function get_dynamic(): dynamic { return false; }
