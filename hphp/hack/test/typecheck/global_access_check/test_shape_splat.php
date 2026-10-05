<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

class ShapeSplatGlobalValue {
  public int $value = 0;
}

class ShapeSplatGlobals {
  <<__LateInit>>
  public static shape('value' => ShapeSplatGlobalValue) $value;
}

function shape_splat_sink(mixed $_): void {}

function test_shape_splat_global_access(): void {
  $copy = shape(...ShapeSplatGlobals::$value);
  shape_splat_sink($copy);
}
