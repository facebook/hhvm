<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): void {
  ExampleDsl`{
    $f = (ExampleShape<shape('x' => ExampleInt, ?'y' => ExampleString)> $shape) ==> 3;
    $f(shape('x' => 2, 'y' => 'hi'));
    $f(shape('x' => 2));
  }`;
}
