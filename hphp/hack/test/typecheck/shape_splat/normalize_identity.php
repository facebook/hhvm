<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type T = shape('x' => int, 'y' => string);

// shape() is identity for merge
type TWithEmpty = shape(...shape(), ...T);
type TEmptyRight = shape(...T, ...shape());

function test_left(TWithEmpty $s): void {
  hh_expect<shape('x' => int, 'y' => string)>($s);
  hh_expect<int>($s['x']);
}

function test_right(TEmptyRight $s): void {
  hh_expect<shape('x' => int, 'y' => string)>($s);
  hh_expect<int>($s['x']);
}
