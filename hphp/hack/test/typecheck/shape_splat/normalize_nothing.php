<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Spreading `nothing` into a shape yields the bottom row, so the whole shape
// collapses to `nothing`. Adjacent bottom rows must be absorbed silently: a
// second `...nothing` after the accumulator is already bottom must not
// spuriously report "only shapes can be unpacked".

function single(shape(...nothing) $s): void {
  hh_expect<nothing>($s);
}

function bottom_then_field(shape(...nothing, 'x' => int) $s): void {
  hh_expect<nothing>($s);
}

function bottom_then_nothing(shape(...nothing, ...nothing) $s): void {
  hh_expect<nothing>($s);
}
