<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

// Rightmost-wins for reads through a concrete splat. Found dogfooding the Bloks
// style shapes in www: `Shapes::idx` returned the BASE field type rather than
// the override, and merely having one colliding field degraded idx on the
// OTHER, non-colliding fields to `nonnull`.

type Base = shape('position' => string, 'align' => int);

// A required right field overrides outright.
function required_override(shape(...Base, 'position' => bool) $s): void {
  hh_expect<?bool>(Shapes::idx($s, 'position'));
  hh_expect<?int>(Shapes::idx($s, 'align'));
}

// An optional right field does not override; it unions with the left one and
// keeps the left's requiredness.
function optional_override(shape(...Base, ?'position' => bool) $s): void {
  hh_expect<?(string | bool)>(Shapes::idx($s, 'position'));
  hh_expect<?int>(Shapes::idx($s, 'align'));
}
