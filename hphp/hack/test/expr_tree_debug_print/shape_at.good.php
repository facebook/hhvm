<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_shape_creation')>>

function g(): void {
  ExampleDsl`{
    return ExampleDsl::shapeAt(shape('x' => 3, 'y' => 'hello'), 'y');
  }`;
}
