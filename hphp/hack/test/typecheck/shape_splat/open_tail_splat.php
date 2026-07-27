<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// A splat shape with a trailing open `...` must be OPEN. Regression: the direct
// decl parser used to drop the trailing `...` when the shape contained splats
// (folding openness only into field-runs), so `shape(...Base, ...)` decl'd as
// closed `shape(?'a' => int)` and would not absorb extra fields.
type Base = shape(?'a' => int);
type OpenSplat = shape(...Base, ...);
type Extended = shape(?'a' => int, ?'b' => string);

// The open tail absorbs the extra field `'b'`, so `Extended <: OpenSplat`.
function into_splat(Extended $e)[]: OpenSplat {
  return $e;
}

// The normalized form is the open `shape(?'a' => mixed, ...)`.
function show(OpenSplat $s): void {
  hh_show($s);
}
