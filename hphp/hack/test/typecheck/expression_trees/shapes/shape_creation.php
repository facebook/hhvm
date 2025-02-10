<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

type MyExampleShape = ExampleShape<shape('y' => ExampleString)>;

function g(): void {
  ExampleDsl`{
    $f = (ExampleShape<shape('x' => ExampleInt)> $shape) ==> 3;
    $f(shape('x' => 2));
  }`;

  ExampleDsl`{
    $f = (ExampleShape<shape('x' => ExampleInt)> $shape) ==> 3;
    $x = shape('x' => 2);
    $f($x);
  }`;

  ExampleDsl`{
    $f = (MyExampleShape $shape) ==> 3;
    $x = shape('y' => 'hello');
    $f($x);
  }`;
}
