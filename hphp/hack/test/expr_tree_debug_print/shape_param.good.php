<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_shape_creation')>>

function g(): void {
  ExampleDsl`{
    $f = (ExampleShape<shape('x' => ExampleInt, 'y' => ExampleString)> $shape) ==> 3;
    return $f(shape('x' => 3, 'y' => 'hello'));
  }`;
}
