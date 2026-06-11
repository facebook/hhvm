<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function foo(): void {
  $shape = shape('x' => 1, 'y' => 'hello');

  // Bare Hack shapes autolift into expression trees via the
  // RepresentableAs<dict<arraykey, ?ExampleAutoLiftable>> arm of
  // ExampleAutoLiftable, mirroring BKS2's BKSAutoLiftable change.
  $y = ExampleDsl`${$shape}`;
}
