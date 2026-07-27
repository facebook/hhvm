<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Base = shape('x' => int, 'y' => string);
type WithAbsent = shape(?'x' => nothing);

type Result = shape(...Base, ...WithAbsent);

function test_absent_noop(Result $r): void {
  hh_expect<int>($r['x']);
  hh_expect<string>($r['y']);
}
