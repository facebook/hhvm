<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

newtype Bad = shape(...Left, ...Right);
