<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Readonly propagation through shape splats
class Box {
  public function __construct(
    public shape('x' => int) $data,
  ) {}
}

function test_readonly(readonly Box $b): void {
  // Splatting a readonly shape field
  $s = shape(...$b->data, 'y' => 'hello');
}
