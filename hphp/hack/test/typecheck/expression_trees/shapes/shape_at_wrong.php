<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

type MyExampleShape = ExampleShape<shape('y' => ExampleString)>;

// wrong return type
function f(): ExampleDslExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    return ExampleDsl::shapeAt($x, 'y');
  }`;
}

function g(): void {
  ExampleDsl`(MyExampleShape $shape) ==> {
    // Too many arguments
    ExampleDsl::shapeAt($shape, 'y', 'y');

    // Not a literal key
    ExampleDsl::shapeAt($shape, ''.'y');

    // Too few args
    ExampleDsl::shapeAt($shape);

    // Non existing field
    ExampleDsl::shapeAt($shape, 'x');

    // Not a shape
    ExampleDsl::shapeAt('y', 'y');
  }`;
}
