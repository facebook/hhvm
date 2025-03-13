<?hh

<<file:
  __EnableUnstableFeatures(
    'expression_trees',
    'expression_tree_shape_creation',
  )>>

type MyExampleShape = ExampleShape<shape('y' => ExampleString)>;

function f1(): ExampleDslExpression<?ExampleString> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    return ExampleDsl::shapeIdx($x, 'y');
  }`;
}

function f2(): ExampleDslExpression<ExampleString> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    return ExampleDsl::shapeIdx($x, 'y', 'test');
  }`;
}

function f27(): ExampleDslExpression<ExampleString> {
  return ExampleDsl`{
    return ExampleDsl::shapeIdx(null, 'y', 'test');
  }`;
}

function f3(): ExampleDslExpression<ExampleString> {
  return ExampleDsl`{
    // Note: we don't support open shapes here yet, as we don't have a way to convert mixed => ExampleMixed without type unification
    $f = (ExampleShape<shape(?'y' => ExampleString)> $shape) ==> ExampleDsl::shapeIdx($shape, 'y', 'default');
    return $f(shape());
  }`;
}

function f4(): ExampleDslExpression<?ExampleString> {
  return ExampleDsl`{
    $f = (?ExampleShape<shape(?'y' => ExampleString)> $shape) ==> ExampleDsl::shapeIdx($shape, 'y');
    return $f(shape());
  }`;
}

function f5(): ExampleDslExpression<ExampleString> {
  return ExampleDsl`{
    $f = (?ExampleShape<shape(?'y' => ExampleString)> $shape) ==> ExampleDsl::shapeIdx($shape, 'y', 'default');
    return $f(shape());
  }`;
}

function g(): ExampleDslExpression<ExampleString> {
  return ExampleDsl`{
    $f = (MyExampleShape $shape) ==> ExampleDsl::shapeIdx($shape, 'y', 'default');
    return $f(shape('y' => 'test2'));
  }`;
}

function h(): ExampleDslExpression<?ExampleString> {
  // nullable  field
  return ExampleDsl`{
    $f = (ExampleShape<shape('y' => ?ExampleString)> $shape) ==>
      ExampleDsl::shapeIdx($shape, 'y');
    return $f(shape('y' => 'test2'));
  }`;
}


function h2(): ExampleDslExpression<?ExampleString> {
  // nullable  field. A provided null field take precedence over the default value,
  // so this can return null.
  return ExampleDsl`{
    $f = (ExampleShape<shape('y' => ?ExampleString)> $shape) ==>
      ExampleDsl::shapeIdx($shape, 'y', 'test');
    return $f(shape('y' => null));
  }`;
}
