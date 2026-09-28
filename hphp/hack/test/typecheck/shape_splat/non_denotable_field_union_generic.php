<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right<T> = shape(?'field' => T);

type Bad<T> = shape(...Left, ...Right<T>);
