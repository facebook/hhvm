<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// When the input row comes from another generic parameter, `tail_of` infers an
// open row that excludes the explicit `x` field.
function tail_of<T1 as shape(...)>(shape(...T1, 'x' => int) $in): T1 {
  throw new Exception();
}

function from_generic<T as shape(...)>(shape(...T, 'x' => int) $s): void {
  hh_expect<shape(absent 'x', ...)>(tail_of($s));
}
