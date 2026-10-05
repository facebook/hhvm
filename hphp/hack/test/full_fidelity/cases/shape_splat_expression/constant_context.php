<?hh
<<file:__EnableUnstableFeatures('shape_splat_expression')>>

const GOOD = shape(...shape('x' => 1));
const BAD = shape(...not_constant());
