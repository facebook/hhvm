<?hh

<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// `absent 'y'` desugars to an optional `nothing` field. The synthesized
// `nothing`'s witness position should span the whole `absent 'y'` field,
// matching the lowerer (not just the `absent` keyword).
type TAbsent = shape(
  'x' => int,
  absent 'y',
);
