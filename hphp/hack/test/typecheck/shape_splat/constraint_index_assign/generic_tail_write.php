<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The directory's HH_FLAGS sets constraint_array_index_assign, which is what
// reaches the Tcan_index_assign -> Shape_splat arm. The row is `[T, shape('a' =>
// int)]`, so under rightmost-wins set_rightmost_field puts the write on the
// concrete element past the generic; the opaque `T` to its left must not absorb
// it. Sibling of new_field.php / overwrite_existing.php, neither of which has a
// generic in the row.
//
// CAVEAT: that constraint feature leaves post-assignment types unresolved even
// for plain simple shapes, so the hh_expect may not be observable yet -- the
// value here is reachability plus no crash.

function write_past_generic<T as shape(...)>(shape(...T, 'a' => int) $s): void {
  $s['b'] = 'new';
  hh_expect<string>($s['b']);
}
