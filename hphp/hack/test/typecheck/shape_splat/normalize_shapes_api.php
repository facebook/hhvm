<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'union_intersection_type_hints')>>

type TBase = shape('x' => int, 'y' => string);
type TExtended = shape(...TBase, 'z' => bool);

function test_idx(TExtended $s): void {
  $x = Shapes::idx($s, 'x');
  hh_expect<?int>($x);
}

function test_at(TExtended $s): void {
  $x = Shapes::at($s, 'x');
  hh_expect<int>($x);
}

function test_remove_key(TExtended $s): void {
  Shapes::removeKey(inout $s, 'x');
  hh_expect<shape('y' => string, 'z' => bool)>($s);
}

function test_to_dict(TExtended $s): void {
  $d = Shapes::toDict($s);
  hh_expect<dict<string, (int | bool | string)>>($d);
}
