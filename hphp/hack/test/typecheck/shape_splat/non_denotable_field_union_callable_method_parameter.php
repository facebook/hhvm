<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right<T> = shape(?'field' => T);

abstract class C<T> {
  abstract public function bad_parameter(
    shape(...Left, ...Right<T>) $_,
  ): void;
}
