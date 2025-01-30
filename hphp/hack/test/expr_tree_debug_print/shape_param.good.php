<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_shape_creation')>>

function g(): void {
  ExampleDsl`{
    $f = (shape('x' => ExampleInt, 'y' => ExampleString) $shape) ==> 3;
    return $f(shape('x' => 3, 'y' => 'hello'));
  }`;
}
