<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A splat of one element and nothing else IS that element, so normalization
// lifts it out: `shape(...T)` is `T`, with no residual splat wrapper.
function lifted<T as shape('a' => int)>(shape(...T) $s): void {
  hh_expect_equivalent<T>($s);
  // Reached through the ordinary rules for `T`, not the corner's bound
  // projection, so the bound's fields are readable.
  hh_expect<int>($s['a']);
}

// Two elements still need the wrapper.
function not_lifted<T as shape('a' => int)>(shape(...T, 'b' => bool) $s): void {
  hh_expect_equivalent<shape(...T, 'b' => bool)>($s);
  hh_expect<bool>($s['b']);
}

// The lifted and unlifted spellings must decide subtyping the same way, and
// both must REJECT: the super requires 'b' and `T` cannot supply it.
function wrapped_sub<T as shape()>(shape(...T) $s): shape(...T, 'b' => int) {
  return $s;
}

function bare_sub<T as shape()>(T $s): shape(...T, 'b' => int) {
  return $s;
}

// Same pair with `T` pinned from both sides, which is how
// shapeSplatOracleTest's prop_rigid_param exercises it.
function wrapped_sub_pinned<T super shape() as shape()>(
  shape(...T) $s,
): shape(...T, 'b' => int) {
  return $s;
}

function bare_sub_pinned<T super shape() as shape()>(
  T $s,
): shape(...T, 'b' => int) {
  return $s;
}
