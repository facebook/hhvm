<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

type MyExampleShape = ExampleShape<shape('y' => ExampleString)>;

// known to not exists
function a(): ExampleDslExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    return ExampleDsl::shapeIdx($x, 'z');//
  }`;
}

// known to not exists
function b(): ExampleDslExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    return ExampleDsl::shapeIdx($x, 'z', 3);
  }`;
}


// wrong return type
function f(): ExampleDslExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    return ExampleDsl::shapeIdx($x, 'y');
  }`;
}

// wrong return type
function f2(): ExampleDslExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape();
    return ExampleDsl::shapeIdx($x, 'y', 'test');
  }`;
}

function g(): void {
  ExampleDsl`(MyExampleShape $shape) ==> {
    // Too many arguments
    ExampleDsl::shapeIdx($shape, 'x', 'y', 3);//

    // Not a literal key
    ExampleDsl::shapeIdx($shape, ''.'y');//

    // Too few args
    ExampleDsl::shapeIdx($shape);//

    // Non existing field
    ExampleDsl::shapeIdx($shape, 'x');

    // Not a shape
    ExampleDsl::shapeIdx('y', 'y');
  }`;
}

function j(): ExampleDslExpression<ExampleString> {
  // open shapes are  not supported, as we can't lift the type of mixed => ExampleMixed yet,
  // until type unification is implemented
  return ExampleDsl`{
    $f = (ExampleShape<shape(...)> $shape) ==> ExampleDsl::shapeIdx($shape, 'y', 'test');
    return $f(shape());
  }`;
}
