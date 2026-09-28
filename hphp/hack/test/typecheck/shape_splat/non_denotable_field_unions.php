<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape(
  'nested' => vec<bool>,
  'required_optional' => bool,
  ?'optional_optional' => float,
  'three_way' => bool,
);
type Middle = shape(?'three_way' => string);
type Right = shape(
  ?'nested' => vec<string>,
  ?'required_optional' => string,
  ?'optional_optional' => bool,
  ?'three_way' => float,
);

type Bad = shape(...Left, ...Middle, ...Right);
