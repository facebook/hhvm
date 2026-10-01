<?hh
<<file:__EnableUnstableFeatures('like_type_hints', 'shape_splat_concrete')>>

type HasX = shape('x' => int);

// REGRESSION (previously a false negative): a like-type wrapper must not hide
// an overlapping splat.
<<__DisjointShapeSplat>>
type Bad = ~shape(...HasX, 'x' => bool);

// SOUND ACCEPTANCE CONTROL: the inner shape is internally disjoint.
<<__DisjointShapeSplat>>
type Good = ~shape(...shape(), 'x' => int);
