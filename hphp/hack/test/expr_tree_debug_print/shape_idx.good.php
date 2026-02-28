<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_shape_creation')>>

function g(): void {
  ExampleDsl`{
    ExampleDsl::shapeIdx(shape('x' => 3), 'x');
    ExampleDsl::shapeIdx(shape('x' => 3), 'y', 4);
  }`;
}
