<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

function bad_variadic_parameter(shape(...Left, ...Right) ...$_): void {}
