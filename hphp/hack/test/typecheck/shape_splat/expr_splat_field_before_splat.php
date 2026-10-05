<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// A concrete field appearing BEFORE a splat. Under rightmost-wins the splat
// overrides an overlapping leading field -- the opposite override direction
// from `shape(...$splat, 'field' => ...)`.
function test_field_then_splat(shape('x' => string, 'z' => bool) $foo): void {
  $r = shape('x' => 2, 'y' => 'hi', ...$foo);
  hh_expect<shape('x' => string, 'y' => string, 'z' => bool)>($r);
  hh_expect<string>($r['x']); // splat's 'x' (string) overrides the literal int
  hh_expect<string>($r['y']);
  hh_expect<bool>($r['z']);
}

// Fields and splats interleaved: field, then splat, then field.
function test_interleaved(shape('m' => bool) $foo): void {
  $r = shape('a' => 1, ...$foo, 'b' => 'hi');
  hh_expect<shape('a' => int, 'b' => string, 'm' => bool)>($r);
  hh_expect<int>($r['a']);
  hh_expect<bool>($r['m']);
  hh_expect<string>($r['b']);
}
