<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Field access on Shape_splat where element is Tgeneric (not Tvar)
// Exercises resolve_elem for Tgeneric with upper bounds
function access_generic_field<T as shape('id' => int, ...)>(
  shape(...T, 'name' => string) $s,
): void {
  // 'name' is in the concrete part — should resolve to string
  hh_expect<string>($s['name']);
  // 'id' is in T's bound — should resolve through upper bounds
  hh_expect<shape(...T, 'name' => string)>($s);
}
