<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TBase = shape('x' => int, 'y' => string);
type TExtended = shape(...TBase, 'z' => bool);

function test_idx(TExtended $s): void {
  $x = Shapes::idx($s, 'x');
  hh_show($x);
}

function test_at(TExtended $s): void {
  $x = Shapes::at($s, 'x');
  hh_show($x);
}

function test_remove_key(TExtended $s): void {
  $s2 = Shapes::removeKey(inout $s, 'x');
  hh_show($s);
}

function test_to_dict(TExtended $s): void {
  $d = Shapes::toDict($s);
  hh_show($d);
}
