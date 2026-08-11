<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_shape_creation',
    'expression_tree_subscript',
  )>>

type MyShapeSubscript = ExampleShape<shape('y' => ExampleString)>;

// Shape field read by subscript on a locally created shape.
function test_shape_subscript_local(): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    return $x['y'];
  }`;
}

// Shape field read by subscript on a lambda parameter.
function test_shape_subscript_param(): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $f = (MyShapeSubscript $shape) ==> $shape['y'];
    return $f(shape('y' => 'test2'));
  }`;
}

// The read resolves to the individual field's type, not a join over all
// fields -- returning ExampleInt here only type-checks if 'i' is selected.
function test_shape_subscript_picks_field_type(
): ExampleExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape('s' => 'test', 'i' => 1);
    return $x['i'];
  }`;
}

// Nested shape reads compose.
function test_shape_subscript_nested(): ExampleExpression<ExampleString> {
  return ExampleDsl`{
    $x = shape('inner' => shape('y' => 'test'));
    return $x['inner']['y'];
  }`;
}

// A shape read feeding an arithmetic operator: this only type-checks if the
// field type survives, rather than degrading to mixed.
function test_shape_subscript_in_expression(): ExampleExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape('i' => 1);
    return $x['i'] + 1;
  }`;
}
