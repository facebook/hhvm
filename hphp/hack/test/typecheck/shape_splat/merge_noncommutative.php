<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type L = shape('x' => int);
type R = shape('x' => string);

type LR = shape(...L, ...R);
type RL = shape(...R, ...L);

function test_noncommutative(LR $lr, RL $rl): void {
  hh_expect<string>($lr['x']);
  hh_expect<int>($rl['x']);
}
