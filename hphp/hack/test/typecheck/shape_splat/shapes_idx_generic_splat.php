<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Shapes::idx / Shapes::at on a generic splat shape.
function read<T as shape('a' => int)>(shape(...T) $s): void {
  hh_expect<?int>(Shapes::idx($s, 'a'));
  hh_expect<int>(Shapes::at($s, 'a'));
}
