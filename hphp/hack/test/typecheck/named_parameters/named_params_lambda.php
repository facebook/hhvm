<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>

function test_lambda(): void {
  $lambda = (int $x, named string $opt, named vec<int> $items = vec[]): int ==> {
    return $x + $items[0];
  };

  // Valid calls
  $lambda(1, opt="test", items=vec[1, 2, 3]);
  $lambda(1, opt="test");

  // Missing required named parameter
  $lambda(1);

  // Extra named parameter
  $lambda(1, opt="test", items=vec[1], extra=123);
}
