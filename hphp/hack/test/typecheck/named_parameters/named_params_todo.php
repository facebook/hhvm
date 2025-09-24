<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

// TODO: Features that need to be implemented for named parameters

function take_many(named int $n, int $x1, int $x2, int $x3): void {}

function test_splat_before_named(): void {
  $numbers = tuple(2, 3, 5);

  // TODO(named_params): This should be allowed but currently produces parsing errors:
  take_many(...$numbers, n=1);
}

class Example { public function __construct(int $a, named string $name): void {} }

function test_constructor_named_args(): void {
  // TODO: Constructor calls with named arguments show type mismatches
  // indicating the named parameter syntax isn't being parsed correctly in constructor contexts

  // TODO(named_params): should work but currently shows "Invalid argument" errors:
  $obj = new Example(1, name="test"); // Shows type mismatch instead of working
}
