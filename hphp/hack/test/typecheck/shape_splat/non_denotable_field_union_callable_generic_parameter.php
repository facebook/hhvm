<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right<T> = shape(?'field' => T);

function bad_generic_parameter<T>(shape(...Left, ...Right<T>) $_): void {}
