<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_shape_creation',
  )>>

type MyExampleShape = ExampleShape<shape('y' => ExampleString)>;

function h(): ExampleDslExpression<ExampleString> {
  return ExampleDsl`{
    $f = (MyExampleShape $shape) ==> ExampleDsl::shapeAt($shape, 'y');
    $x = shape();
    $x = ExampleDsl::shapePut($x, 'y', 'test3');
    return $f($x);
  }`;
}
