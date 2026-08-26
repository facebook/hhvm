<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Base = shape('x' => int, 'y' => string);
type WithAbsent = shape(absent 'x');

// absent doesn't 'remove' x under merge
type Result = shape(...Base, ...WithAbsent);

function test_absent_noop(Result $r): void {
  hh_expect<shape('x' => int, 'y' => string)>($r);
}
