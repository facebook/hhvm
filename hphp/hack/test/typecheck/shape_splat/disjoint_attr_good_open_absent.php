<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type OpenWithoutX = shape(absent 'x', ...);

// OpenWithoutX cannot supply 'x', so these components are disjoint.
<<__DisjointShapeSplat>>
type ShouldBeValid = shape(...OpenWithoutX, 'x' => int);
