<?hh

class Example { public function __construct(int $x, named string $y): void {} }

function test_constructor_named_args(): void {
  // TODO: Constructor calls with named arguments show type mismatches
  // indicating the named parameter syntax isn't being parsed correctly in constructor contexts
  new Example(y="", 1);
  new Example(1, y="");

  new Example(1, y=true); // Error
  new Example(1, y="", extra_param=""); // Error
  new Example(1); // Error
}
