<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

function test(num $x): void {
  if (coinflip()) $x = get_dynamic(); // $x: (num | dynamic)
  match ($x) {
    _: int => hh_expect<int>($x);
    _: float => hh_expect<float>($x);
  }

  $x = get_dynamic(); // $x: dynamic
  match ($x) {
    _: string => hh_expect<string>($x);
    // considered exhaustive because $x: dynamic
  }
}

function coinflip(): bool { return false; }
function get_dynamic(): dynamic { return false; }
