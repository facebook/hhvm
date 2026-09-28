<?hh
<<file:__EnableUnstableFeatures('case_types', 'shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

case type Bad = shape(...Left, ...Right);
