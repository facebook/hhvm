<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_shape_creation',
  )>>

type MyExampleShape = ExampleShape<shape('y' => ExampleString)>;

function f(): ExampleDslExpression<ExampleString> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    return ExampleDsl::shapeAt($x, 'y');
  }`;
}

function g(): ExampleDslExpression<ExampleString> {
  return ExampleDsl`{
    $f = (MyExampleShape $shape) ==> ExampleDsl::shapeAt($shape, 'y');
    return $f(shape('y' => 'test2'));
  }`;
}

function h(): ExampleDslExpression<?ExampleString> {
  // nullable  field
  return ExampleDsl`{
    $f = (ExampleShape<shape('y' => ?ExampleString)> $shape) ==>
      ExampleDsl::shapeAt($shape, 'y');
    return $f(shape('y' => 'test2'));
  }`;
}

function i(): ExampleDslExpression<ExampleString> {
  // optional field
  return ExampleDsl`{
    $f = (ExampleShape<shape(?'y' => ExampleString)> $shape) ==>
      ExampleDsl::shapeAt($shape, 'y');
    return $f(shape());
  }`;
}

function j(): ExampleDslExpression<mixed> {
  // open shape
  return ExampleDsl`{
    $f = (ExampleShape<shape(...)> $shape) ==> ExampleDsl::shapeAt($shape, 'y');
    return $f(shape());
  }`;
}
