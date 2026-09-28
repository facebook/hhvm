<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

function bad_nested_parameter(vec<shape(...Left, ...Right)> $_): void {}
