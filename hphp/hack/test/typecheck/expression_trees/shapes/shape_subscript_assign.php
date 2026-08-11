<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_shape_creation',
    'expression_tree_subscript',
  )>>

type MyShapeAssign = ExampleShape<shape('y' => ExampleString)>;

// Write a field, then read it back.
function test_shape_subscript_assign(): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $x = shape('y' => 'before');
    $x['y'] = 'after';
    return $x['y'];
  }`;
}

// The written type flows to a later read: returning ExampleInt only
// type-checks if 'i' is still an int after the write.
function test_shape_subscript_assign_type_flows(
): ExampleExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape('i' => 1);
    $x['i'] = 2;
    return $x['i'] + 1;
  }`;
}

// Shape assignment updates the field's type, as it does in Hack.
function test_shape_subscript_assign_widens_field(
): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $x = shape('i' => 1);
    $x['i'] = 'now a string';
    return $x['i'];
  }`;
}

// Nested write through two shape levels.
function test_shape_subscript_assign_nested(
): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $x = shape('inner' => shape('y' => 'before'));
    $x['inner']['y'] = 'after';
    return $x['inner']['y'];
  }`;
}

// Write to a shape that arrived as a lambda parameter.
function test_shape_subscript_assign_param(
): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $f = (MyShapeAssign $shape) ==> {
      $shape['y'] = 'assigned';
      return $shape['y'];
    };
    return $f(shape('y' => 'initial'));
  }`;
}

// A write does not have to be followed by a read.
function test_shape_subscript_assign_only(): ExampleExpression<ExampleVoid> {
  return ExampleDsl`{
    $x = shape('y' => 'a');
    $x['y'] = 'b';
  }`;
}
