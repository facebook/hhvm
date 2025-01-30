<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function g(): void {
  ExampleDsl`{
    $f = (shape('x' => ExampleInt, 'y' => ExampleString) $shape) ==> 3;
    $f(shape('x' => 2));
  }`;
}
