<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('x' => int, 'y' => string);
type B = shape('y' => bool, 'z' => float);
type C = shape('z' => string);

type AB = shape(...A, ...B);
type ABC_left = shape(...AB, ...C);

type BC = shape(...B, ...C);
type ABC_right = shape(...A, ...BC);

function test_assoc(ABC_left $left, ABC_right $right): void {
  hh_expect<ABC_right>($left);
  hh_expect<ABC_left>($right);
}
