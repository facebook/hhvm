<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Test parameterized type alias with splat
type TBase<T> = shape('x' => T, 'y' => string);
type TDerived = shape(...TBase<int>, 'z' => bool);

function test(TDerived $s): void {
  hh_expect<shape('x' => int, 'y' => string, 'z' => bool)>($s);
  hh_expect<int>($s['x']);
  hh_expect<string>($s['y']);
  hh_expect<bool>($s['z']);
}
