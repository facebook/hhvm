<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

type MyExampleShape = ExampleShape<shape('y' => ExampleString)>;

// wrong return type
function f(): ExampleDslExpression<ExampleInt> {
  return ExampleDsl`{
    $x = shape('y' => 'test');
    $shape = ExampleDsl::shapePut($x, 'y', true);
    return ExampleDsl::shapeAt($shape, 'y');
  }`;
}

function g(): void {
  ExampleDsl`(MyExampleShape $shape) ==> {
    // Too many arguments
    ExampleDsl::shapePut($shape, 'x', 'y', 'z');

    // Too few args
    ExampleDsl::shapePut($shape);

    // Not a literal key
    ExampleDsl::shapePut($shape, ''.'y', 3);

    // Not a shape
    ExampleDsl::shapePut('y', 'y', 3);
  }`;
}
