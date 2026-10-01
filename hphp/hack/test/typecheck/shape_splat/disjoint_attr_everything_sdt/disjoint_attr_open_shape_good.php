<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// SOUND ACCEPTANCE CONTROL: the automatically wrapped open shape is accepted
// when an empty splat cannot overlap the explicit field.
<<__DisjointShapeSplat>>
type Good = shape(...shape(), 'right' => bool, ...);
