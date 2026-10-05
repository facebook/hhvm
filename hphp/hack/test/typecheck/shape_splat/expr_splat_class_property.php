<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// A shape splat in a property initializer requires deep initialization.
class C {
  public shape('x' => int, 'y' => string) $value =
    shape(...shape('x' => 1), 'y' => 'hello');
}
