//// remove_key_absent_opaque_newtype_def.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

newtype RemoveKeyAbsentBound as shape('kept' => int) = shape('kept' => int);

//// remove_key_absent_opaque_newtype_use.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// removeKey is a no-op on an opaque spread when its closed upper bound proves
// that the removed field is absent.
function remove_key_absent_opaque_newtype(
  shape(...RemoveKeyAbsentBound, 'present' => string) $s,
): void {
  Shapes::removeKey(inout $s, 'missing');
}
