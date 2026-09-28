<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

final class LeftValue {}
final class RightValue {}

type Left = shape('field' => LeftValue);
type Right = shape(?'field' => RightValue);

type Bad = shape(...Left, ...Right);
