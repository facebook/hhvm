<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

<<__EntryPoint>>
function test(): void {
  require __DIR__.'/../../../hack/test/expr_tree.php';

  $shape = shape('x' => 1, 'y' => 'hello');
  print_et(ExampleDsl`${$shape}`);

  // Nested shape: inner shape autolifts recursively via the dict arm.
  $nested = shape('outer' => shape('inner' => 42));
  print_et(ExampleDsl`${$nested}`);

  // Empty shape.
  print_et(ExampleDsl`${shape()}`);
}
