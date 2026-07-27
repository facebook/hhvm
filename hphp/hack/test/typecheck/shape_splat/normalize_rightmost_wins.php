<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TLeft = shape('x' => int, 'y' => string);
type TRight = shape('x' => bool);
// 'x' from TRight should override TLeft's 'x'
type TMerged = shape(...TLeft, ...TRight);

function test(TMerged $s): void {
  hh_expect<shape('x' => bool, 'y' => string)>($s);
  // 'x' should be bool (rightmost wins), 'y' should be string
  hh_expect<bool>($s['x']);
  hh_expect<string>($s['y']);
}
